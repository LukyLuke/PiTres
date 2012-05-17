
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
	sinceDate->setDate(settings.value("sentbills/sincedate", QDate::currentDate()).toDate());
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(btnInvoiceQif, SIGNAL(clicked()), this, SLOT(exportQifAssets()));
	connect(btnSyncUsers, SIGNAL(clicked()), this, SLOT(syncUserPaidDate()));
	connect(pendingOnly, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(sinceDate, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	
	tableModel = new QSqlQueryModel(tableView);
	
	loadData();
	createContextMenu();
	
	adjustDialog = new QDialog(this);
	form.setupUi(adjustDialog);
	connect(form.actionAdjust, SIGNAL(triggered()), this, SLOT(doAdjustDates()));
	
	fromtoDialog = new QDialog(this);
	fromto.setupUi(fromtoDialog);
	connect(fromto.actionChoose, SIGNAL(triggered()), this, SLOT(doExportQifAssets()));
	
	// Enable the ContextMenu
	tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTableContextMenu(const QPoint&)));
}

SentBills::~SentBills() {
	delete actionSendReminder;
	delete actionPrintReminder;
}

void SentBills::createContextMenu() {
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
	QString qs("SELECT * FROM pps_invoice WHERE issue_date >= :issued_after");
	
	if (pendingOnly->isChecked()) {
		qs.append(" AND paid_date=''");
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
	
	QSqlQuery query = createQuery();
	tableModel->setQuery(query);
}

void SentBills::exportQifAssets() {
	QDate now = QDate::currentDate();
	fromto.fromDate->setDate(QDate(now.year()-1, 1, 1));
	fromto.toDate->setDate(QDate(now.year(), 1, 1));
	fromtoDialog->show();
}

void SentBills::doExportQifAssets() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_asset", "Oth A").toString());
	float amountLimited = settings.value("invoice/amount_limited", 30.0).toFloat();
	//float amountDefault = settings.value("invoice/amount_default", 60.0).toFloat();
	
	QSqlQuery query(db);
	query.prepare("SELECT member_uid,reference,amount,issue_date,address_name FROM pps_invoice WHERE issue_date >= :date1 AND issue_date <= :date2;");
	query.bindValue(":date1", fromto.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", fromto.toDate->date().toString("yyyy-MM-dd"));
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
	fromtoDialog->hide();
}

void SentBills::syncUserPaidDate() {
	QDate now = QDate::currentDate();
	form.paidDate->setDate(QDate(now.year(), 1, 1));
	form.paidDueDate->setDate(QDate(now.year()+1, 1, 1));
	adjustDialog->show();
}

void SentBills::doAdjustDates() {
	QSettings settings;
	QDate from = form.paidDate->date();
	QDate due = form.paidDueDate->date();
	
	if (form.checkDatabase->isChecked()) {
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
	
	if (form.checkLdif->isChecked()) {
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
	adjustDialog->hide();
}

void SentBills::showTableContextMenu(const QPoint &point) {
	QMenu menu(tableView);
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


