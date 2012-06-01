
#include "SentBills.h"
#include "data/Person.h"
#include "data/Invoice.h"
#include "data/Reminder.h"
#include "helper/XmlPdf.h"

#include <QSizePolicy>
#include <QWidget>
#include <QTableView>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSettings>
#include <QModelIndex>
#include <QVariant>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QPoint>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeySequence>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QDebug>


SentBills::SentBills(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	QSettings settings;
	pendingOnly->setChecked(settings.value("sentbills/pending", false).toBool());
	sinceDate->setDate(settings.value("sentbills/sincedate", QDate::currentDate().addMonths(-3)).toDate());
	maxDate->setDate(settings.value("sentbills/maxdate", QDate::currentDate().addDays(1)).toDate());
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(btnInvoiceQif, SIGNAL(clicked()), this, SLOT(exportQifAssets()));
	connect(btnSyncUsers, SIGNAL(clicked()), this, SLOT(syncUserPaidDate()));
	connect(pendingOnly, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(sinceDate, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(maxDate, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(btnPaymentsExport, SIGNAL(clicked()), this, SLOT(exportQifPayments()));
	
	tableModel = new QSqlQueryModel(tableView);
	
	loadData();
	createContextMenu();
	
	adjustMemberdateDialog = new QDialog(this);
	adjustMembersDueDateForm.setupUi(adjustMemberdateDialog);
	connect(adjustMembersDueDateForm.actionAdjust, SIGNAL(triggered()), this, SLOT(doAdjustDates()));
	
	invoiceQifDialog = new QDialog(this);
	exportInvoiceQifForm.setupUi(invoiceQifDialog);
	exportInvoiceQifForm.hintLabel->setText(tr("Export all created Invoices, issued in this Daterange, as a QIF to import in GnuCash."));
	connect(exportInvoiceQifForm.actionChoose, SIGNAL(triggered()), this, SLOT(doExportQifAssets()));
	
	paymentQifDialog = new QDialog(this);
	exportPaymentQifForm.setupUi(paymentQifDialog);
	exportPaymentQifForm.hintLabel->setText(tr("Export all Payments, payed in this Daterange, as a QIF to import in GnuCash."));
	connect(exportPaymentQifForm.actionChoose, SIGNAL(triggered()), this, SLOT(doExportQifPayments()));
	
	editDialog = new QDialog(this);
	editInvoice.setupUi(editDialog);
	connect(editInvoice.actionSave, SIGNAL(triggered()), this, SLOT(doEditInvoice()));
	
	// Enable the ContextMenu
	tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTableContextMenu(const QPoint&)));
	connect(tableView, SIGNAL(doubleClicked(QModelIndex)), actionShowEdit, SLOT(trigger()));
}

SentBills::~SentBills() {
	delete actionSendReminder;
	delete actionPrintReminder;
	delete actionShowEdit;
}

void SentBills::createContextMenu() {
	actionShowEdit = new QAction(tr("Edit Invoice"), this);
	actionShowEdit->setStatusTip(tr("Edit the curretly selected Invoice"));
	connect(actionShowEdit, SIGNAL(triggered()), this, SLOT(showEditInvoiceForm()));
	
	actionSendReminder = new QAction(tr("&Send Reminder"), this);
	//actionPrintReminder->setShortcuts(QKeySequence(Qt::CTRL + Qt::Key_R));
	actionSendReminder->setStatusTip(tr("Create a new Reminder and send it by EMail"));
	connect(actionSendReminder, SIGNAL(triggered()), this, SLOT(sendNewReminder()));
	
	actionPrintReminder = new QAction(tr("&Pdf Reminder"), this);
	actionPrintReminder->setShortcuts(QKeySequence::Print);
	actionPrintReminder->setStatusTip(tr("Create a new Reminder and creates a PDF to save and Print it"));
	connect(actionPrintReminder, SIGNAL(triggered()), this, SLOT(printNewReminder()));
}

void SentBills::loadData() {
	// Create Persons Database Tables if needed
	PPSPerson::createTables();
	Invoice::createTables();
	
	tableModel->setQuery(createQuery());
	
	tableModel->setHeaderData(0,  Qt::Horizontal, tr("UID"));
	tableModel->setHeaderData(1,  Qt::Horizontal, tr("Reference"));
	tableModel->setHeaderData(2,  Qt::Horizontal, tr("Issue Date"));
	tableModel->setHeaderData(3,  Qt::Horizontal, tr("Payable Date"));
	tableModel->setHeaderData(4,  Qt::Horizontal, tr("Paid Date"));
	tableModel->setHeaderData(5,  Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(6,  Qt::Horizontal, tr("Amount Paid"));
	tableModel->setHeaderData(7,  Qt::Horizontal, tr("State"));
	tableModel->setHeaderData(8,  Qt::Horizontal, tr("Prefix"));
	tableModel->setHeaderData(9,  Qt::Horizontal, tr("Company"));
	tableModel->setHeaderData(10, Qt::Horizontal, tr("Name"));
	tableModel->setHeaderData(11, Qt::Horizontal, tr("Address"));
	tableModel->setHeaderData(12, Qt::Horizontal, tr("Address 2"));
	tableModel->setHeaderData(13, Qt::Horizontal, tr("PLZ, City"));
	tableModel->setHeaderData(14, Qt::Horizontal, tr("Email"));
	tableModel->setHeaderData(15, Qt::Horizontal, tr("Section"));
	
	tableView->setSortingEnabled(true);
	tableView->setModel(tableModel);
}

void SentBills::timerEvent(QTimerEvent *event) {
	killTimer(searchTimer);
	searchData();
}

void SentBills::searchDataTimeout(QString data) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

QSqlQuery SentBills::createQuery() {
	QSqlQuery query(db);
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	QString qs("SELECT * FROM pps_invoice WHERE issue_date >= :issued_after AND issue_date <= :issued_before");
	
	if (pendingOnly->isChecked()) {
		qs.append(" AND state=" + QString::number((int)Invoice::StateOpen));
	}
	
	if (sl.size() > 0) {
		qs.append(" AND (reference LIKE :reference OR (");
		for (int i = 0; i < sl.size(); i++) {
			if (i > 0) {
				qs.append(" AND ");
			}
			qs.append(QString("(member_uid LIKE :uid_").append(QString::number(i)));
			qs.append(QString(" OR issue_date LIKE :issue_date_").append(QString::number(i)).append(" OR payable_date LIKE :payable_").append(QString::number(i)));
			qs.append(QString(" OR address_name LIKE :name_").append(QString::number(i)).append(" OR address_email LIKE :email_").append(QString::number(i)));
			qs.append(")");
		}
		qs.append("))");
	}
	qs.append(" ORDER BY issue_date,paid_date,state ASC");
	
	query.prepare(qs);
	query.bindValue(":issued_after", sinceDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":issued_before", maxDate->date().toString("yyyy-MM-dd"));
	
	if (sl.size() > 0) {
		query.bindValue(":reference", QString("%%1%").arg(sl.join(" ")));
		for (int i = 0; i < sl.size(); i++) {
			query.bindValue(QString(":uid_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
			query.bindValue(QString(":issue_date_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
			query.bindValue(QString(":payable_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
			query.bindValue(QString(":name_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
			query.bindValue(QString(":email_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		}
	}
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	return query;
}

void SentBills::searchData() {
	QSettings settings;
	settings.setValue("sentbills/pending", pendingOnly->isChecked());
	settings.setValue("sentbills/sincedate", sinceDate->date());
	settings.setValue("sentbills/maxdate", maxDate->date());
	
	QSqlQuery query = createQuery();
	tableModel->setQuery(query);
}

void SentBills::exportQifAssets() {
	QDate now = QDate::currentDate();
	exportInvoiceQifForm.fromDate->setDate(QDate(now.year()-1, 1, 1));
	exportInvoiceQifForm.toDate->setDate(QDate(now.year(), 1, 1));
	invoiceQifDialog->show();
}

void SentBills::exportQifPayments() {
	QDate now = QDate::currentDate();
	exportPaymentQifForm.fromDate->setDate(now.addMonths(-1));
	exportPaymentQifForm.toDate->setDate(now);
	paymentQifDialog->show();
}

void SentBills::doExportQifAssets() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_asset", "Oth A").toString());
	float amountLimited = settings.value("invoice/amount_limited", 30.0).toFloat();
	//float amountDefault = settings.value("invoice/amount_default", 60.0).toFloat();
	
	QSqlQuery query(db);
	query.prepare("SELECT member_uid,reference,amount,issue_date,address_name FROM pps_invoice WHERE issue_date >= :date1 AND issue_date <= :date2;");
	query.bindValue(":date1", exportInvoiceQifForm.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", exportInvoiceQifForm.toDate->date().toString("yyyy-MM-dd"));
	query.exec();
	
	while (query.next()) {
		QString member = query.value(0).toString();
		QString ref = query.value(1).toString();
		QString amount = query.value(2).toString();
		QString valuta = query.value(3).toString();
		QString name = query.value(4).toString();
		
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP" + settings.value("qif/payee_label", tr("Membership: ")).toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", tr("Member UID: ")).toString() + member);
		if (amount.toFloat() > amountLimited) {
			qif.append("\nL" + settings.value("qif/income_limited", "Membership Limited").toString());
		} else {
			qif.append("\nL" + settings.value("qif/income_default", "Membership Default").toString());
		}
		qif.append("\n^\n");
	}
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
	invoiceQifDialog->hide();
}

void SentBills::doExportQifPayments() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_cash", "Cash").toString());
	
	QSqlQuery query(db);
	query.prepare("SELECT member_uid,reference,amount,paid_date,address_name FROM pps_invoice WHERE paid_date >= :date1 AND paid_date <= :date2;");
	query.bindValue(":date1", exportPaymentQifForm.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", exportPaymentQifForm.toDate->date().toString("yyyy-MM-dd"));
	query.exec();
	
	while (query.next()) {
		QString member = query.value(0).toString();
		QString ref = query.value(1).toString();
		QString amount = query.value(2).toString();
		QString valuta = query.value(3).toString();
		QString name = query.value(4).toString();
		
		// Payment QIF
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP"+ settings.value("qif/payee_label", tr("Payment: ")).toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", tr("Member UID: ")).toString() + member);
		qif.append("\nL" + settings.value("qif/account_income", "Membership Fee").toString());
		qif.append("\n^\n");
	}
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
	paymentQifDialog->close();
}

void SentBills::syncUserPaidDate() {
	QDate now = QDate::currentDate();
	adjustMembersDueDateForm.paidDate->setDate(QDate(now.year(), 1, 1));
	adjustMembersDueDateForm.paidDueDate->setDate(QDate(now.year()+1, 1, 1));
	adjustMemberdateDialog->show();
}

void SentBills::doAdjustDates() {
	QSettings settings;
	QDate from = adjustMembersDueDateForm.paidDate->date();
	QDate due = adjustMembersDueDateForm.paidDueDate->date();
	
	if (adjustMembersDueDateForm.checkDatabase->isChecked()) {
		QSqlQuery query(db);
		query.prepare("UPDATE ldap_persons SET paid_due=:paidDueDate WHERE uid IN (SELECT member_uid FROM pps_invoice "
		              "WHERE amount_paid>=amount AND issue_date>=:issueDate AND paid_date>=:paidDate);");
		query.bindValue(":paidDueDate", due.toString("yyyy-MM-dd"));
		query.bindValue(":issueDate", from.toString("yyyy-MM-dd"));
		query.bindValue(":paidDate", from.toString("yyyy-MM-dd"));
		if (!query.exec()) {
			if (query.lastError().type() != QSqlError::NoError) {
				qDebug() << query.lastQuery();
				qDebug() << query.lastError();
			}
			QMessageBox::warning(this, tr("Adjusting dates failed"), tr("An Error occured while adjusting Paid-Dues dates in Database:\n\n%1").arg(query.lastError().text()));
		}
	}
	
	if (adjustMembersDueDateForm.checkLdif->isChecked()) {
		QString ldif = "";
		QString section, member;
		QString duedate = due.toString("yyyy-MM-dd");
		QString members_dn = settings.value("ldif/members_dn", "uniqueIdentifier=%1,dc=members,dc=piratenpartei,dc=ch").toString();
		QString main_dn = settings.value("ldif/main_dn", "uniqueIdentifier=%1,dc=members,st=%2,dc=piratenpartei,dc=ch").toString();
		QString attribute = settings.value("ldif/memberstate_attribute", "ppsVotingRightUntil").toString();
		bool replaceAttribute = settings.value("ldif/replace_attribute", false).toBool();
		
		QSqlQuery query(db);
		query.prepare("SELECT p.uid, p.section FROM pps_invoice i, ldap_persons p WHERE i.amount_paid>=i.amount AND i.issue_date >= :issueDate "
		              "AND i.paid_date >= :paidDate AND i.member_uid=p.uid AND (p.ldap_paid_due < :paidDueDate OR p.ldap_paid_due IS NULL OR p.ldap_paid_due='');");
		query.bindValue(":paidDueDate", duedate);
		query.bindValue(":issueDate", from.toString("yyyy-MM-dd"));
		query.bindValue(":paidDate", from.toString("yyyy-MM-dd"));
		
		if (query.exec()) {
			while (query.next()) {
				section = query.value(1).toString();
				member = query.value(0).toString();
				
				if (section.isEmpty() || section.length() > 2) {
					ldif.append("dn: " + members_dn.arg(member));
				} else {
					ldif.append("dn: " + main_dn.arg(member, section));
				}
				ldif.append("\nchangetype: modify");
				if (replaceAttribute) {
					ldif.append("delete: ");
					ldif.append(attribute);
					ldif.append("\n-\n");
				}
				ldif.append("\nadd: ");
				ldif.append(attribute);
				ldif.append("\n");
				ldif.append(attribute).append(": ").append(duedate);
				ldif.append("\n\n");
			}
			
			// Save the LDIF
			if (ldif.length() > 0) {
				QString fileName = QFileDialog::getSaveFileName(this, tr("Save LDIF File"), "", tr("LDIF (*.ldif);;Plaintext (*.txt)"));
				if (!fileName.isEmpty()) {
					QFile f(fileName);
					if (f.open(QFile::WriteOnly | QFile::Truncate)) {
						QTextStream out(&f);
						out << ldif;
					} else {
						QMessageBox::warning(this, tr("Adjusting dates failed"), tr("An Error occured while opening the LDIF-File.\n\nDo you have Write-Permission to\n%1?").arg(fileName));
					}
				}
			} else {
				QMessageBox::warning(this, tr("Adjusting dates failed"), tr("No Datasets found which would need an Update..."));
			}
		} else {
			if (query.lastError().type() != QSqlError::NoError) {
				qDebug() << query.lastQuery();
				qDebug() << query.lastError();
			}
			QMessageBox::warning(this, tr("Adjusting dates failed"), tr("An Error occured while getting Paid-Dues dates from Database:\n\n%1").arg(query.lastError().text()));
		}
	}
	adjustMemberdateDialog->hide();
}

void SentBills::showTableContextMenu(const QPoint &point) {
	QMenu menu(tableView);
	menu.addAction(actionShowEdit);
	menu.addAction(actionSendReminder);
	menu.addAction(actionPrintReminder);
	menu.exec(tableView->mapToGlobal(point));
}

QSet<int> SentBills::getSelectedRows() {
	QModelIndexList selection = tableView->selectionModel()->selectedIndexes();
	QSet<int> rows;
	for (int i = 0; i < selection.size(); i++) {
		rows << selection.at(i).row();
	}
	return rows;
}

void SentBills::showEditInvoiceForm() {
	QModelIndexList selection = tableView->selectionModel()->selectedIndexes();
	if (selection.size() <= 0) return;
	QSqlRecord record = tableModel->record(selection.at(0).row());
	editInvoice.editReference->setText(record.value("reference").toString());
	editInvoice.editAmount->setValue(record.value("amount").toDouble());
	editInvoice.editPaid->setValue(record.value("amount_paid").toDouble());
	editInvoice.editPaidDate->setDate(record.value("paid_date").toDate());
	editInvoice.editInvoiceDate->setDate(record.value("issue_date").toDate());
	editInvoice.selectState->setCurrentIndex(record.value("state").toInt());
	editDialog->show();
}

void SentBills::doEditInvoice() {
	QModelIndexList selection = tableView->selectionModel()->selectedIndexes();
	if (selection.size() <= 0) return;
	QSqlRecord record = tableModel->record(selection.at(0).row());
	QString old_reference = record.value("reference").toString();
	QSqlQuery query(db);
	query.prepare("UPDATE pps_invoice SET reference=:reference, amount=:amount, amount_paid=:amount_paid, paid_date=:paid_date, issue_date=:issue_date, state=:state WHERE reference=:old_reference;");
	query.bindValue(":reference", editInvoice.editReference->text());
	query.bindValue(":amount", editInvoice.editAmount->value());
	query.bindValue(":amount_paid", editInvoice.editPaid->value());
	query.bindValue(":paid_date", editInvoice.editPaidDate->date());
	query.bindValue(":issue_date", editInvoice.editInvoiceDate->date());
	query.bindValue(":old_reference", old_reference);
	query.bindValue(":state", editInvoice.selectState->currentIndex());
	
	if (!query.exec()) {
		if (query.lastError().type() != QSqlError::NoError) {
			qDebug() << query.lastQuery();
			qDebug() << query.lastError();
		}
		QMessageBox::warning(this, tr("Editing Invoice failed"), tr("An Error occured while editing the selected Invoice:\n\n%1").arg(query.lastError().text()));
	}
	searchData();
	editDialog->hide();
}

void SentBills::sendNewReminder() {
	createPdfReminds(true);
}

void SentBills::printNewReminder() {
	createPdfReminds(false);
}

void SentBills::createPdfReminds(bool email) {
	int uid;
	QString fileName;
	Reminder reminder;
	XmlPdf *pdf;
	QSet<int> rows = getSelectedRows();
	
	if (!email) {
		fileName = QFileDialog::getSaveFileName(this, tr("Save PDF-File"), "", tr("PDF (*.pdf)"));
		if (!fileName.isEmpty()) {
			fileName.replace(QRegExp("\\.pdf$"), "_%1.pdf");
		}
	}
	foreach(const int &i, rows) {
		uid = tableModel->record(i).value(0).toInt();
		reminder.loadLast(uid);
		if (email) {
			pdf = reminder.createPdf("invoice");
			pdf->send(reminder.addressEmail());
		} else {
			pdf = reminder.createPdf("reminder");
			pdf->print( QString(fileName).arg(reminder.memberUid()) );
		}
	}
}


