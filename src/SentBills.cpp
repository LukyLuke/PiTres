/*
	Show all sent Bills and let the User remember the payer or edit the invoice
	Copyright (C) 2012  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SentBills.h"

SentBills::SentBills(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	QSettings settings;
	checkOpposite->setChecked(settings.value("sentbills/search_opposite", false).toBool());
	checkOpen->setChecked(settings.value("sentbills/search_pending", false).toBool());
	checkPaid->setChecked(settings.value("sentbills/search_paid", false).toBool());
	checkCanceled->setChecked(settings.value("sentbills/search_cancelled", false).toBool());
	checkContributed->setChecked(settings.value("sentbills/search_contributed", false).toBool());
	sinceDate->setDate(settings.value("sentbills/sincedate", QDate::currentDate().addMonths(-3)).toDate());
	maxDate->setDate(settings.value("sentbills/maxdate", QDate::currentDate().addDays(1)).toDate());

	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout()));
	connect(btnInvoiceQif, SIGNAL(clicked()), this, SLOT(exportQifAssets()));
	connect(checkOpposite, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(checkOpen, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(checkPaid, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(checkCanceled, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(checkContributed, SIGNAL(toggled(bool)), this, SLOT(searchData()));
	connect(sinceDate, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(maxDate, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(btnPaymentsExport, SIGNAL(clicked()), this, SLOT(exportQifPayments()));
	connect(btnExportCsv, SIGNAL(clicked()), this, SLOT(exportCsvList()));
	
	tableModel = new QSqlQueryModel(tableView);
	loadData();
	createContextMenu();
	createMassChangeButton();

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
	
	csvDialog = new QDialog(this);
	csvExport.setupUi(csvDialog);
	connect(csvExport.actionChoose, SIGNAL(triggered()), this, SLOT(doExportCsvList()));

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

void SentBills::createMassChangeButton() {
	actionStateChange = new QActionGroup(this);

	QAction *open = new QAction(tr("Open"), this);
	open->setStatusTip(tr("Change the paid-state to open."));
	open->setData(QVariant(Invoice::StateOpen));
	actionStateChange->addAction(open);

	QAction *canceled = new QAction(tr("Canceled"), this);
	canceled->setStatusTip(tr("Change the paid-state to canceled."));
	canceled->setData(QVariant(Invoice::StateCanceled));
	actionStateChange->addAction(canceled);

	QAction *paid = new QAction(tr("Paid"), this);
	paid->setStatusTip(tr("Change the paid-state to paid."));
	paid->setData(QVariant(Invoice::StatePaid));
	actionStateChange->addAction(paid);

	QAction *unknown = new QAction(tr("Unknown"), this);
	unknown->setStatusTip(tr("Change the paid-state to unknown."));
	unknown->setData(QVariant(Invoice::StateUnknown));
	actionStateChange->addAction(unknown);

	QAction *contributed = new QAction(tr("Contributed"), this);
	contributed->setStatusTip(tr("Change the paid-state to contributed."));
	contributed->setData(QVariant(Invoice::StateContributed));
	actionStateChange->addAction(contributed);

	connect(actionStateChange, SIGNAL(triggered(QAction*)), this, SLOT(massChangeState(QAction*)));

	QMenu *menu = new QMenu(btnChangeStates);
	menu->addAction(open);
	menu->addAction(canceled);
	menu->addAction(paid);
	menu->addAction(contributed);
	menu->addAction(unknown);
	btnChangeStates->setMenu(menu);
}

void SentBills::loadData() {
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

void SentBills::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	searchData();
}

void SentBills::searchDataTimeout() {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

QSqlQuery SentBills::createQuery() {
	QSqlQuery query(db);
	QString search_operator( checkOpposite->isChecked() ? " <> " : " = " );
	QString search_connector( checkOpposite->isChecked() ? " AND " : " OR " );
	QStringList search_tags = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	QString qs("SELECT * FROM pps_invoice WHERE issue_date >= :issued_after AND issue_date <= :issued_before");
	QString state_query;

	// Get all open invoices
	if (checkOpen->isChecked()) {
		if (!state_query.isEmpty()) {
			state_query.append(search_connector);
		}
		state_query.append("state" + search_operator + QString::number((int)Invoice::StateOpen));
	}

	// Get all paid invoices
	if (checkPaid->isChecked()) {
		if (!state_query.isEmpty()) {
			state_query.append(search_connector);
		}
		state_query.append("state" + search_operator + QString::number((int)Invoice::StatePaid));
	}

	// Get all canceled invoices
	if (checkCanceled->isChecked()) {
		if (!state_query.isEmpty()) {
			state_query.append(search_connector);
		}
		state_query.append("state" + search_operator + QString::number((int)Invoice::StateCanceled));
	}

	// Get all contributed invoices
	if (checkContributed->isChecked()) {
		if (!state_query.isEmpty()) {
			state_query.append(search_connector);
		}
		state_query.append("state" + search_operator + QString::number((int)Invoice::StateContributed));
	}

	// Append the state query
	if (!state_query.isEmpty()) {
		qs.append(" AND (" + state_query + ")");
	}

	// Append all search keywords
	if (search_tags.size() > 0) {
		qs.append(" AND (reference LIKE :reference OR (");
		for (int i = 0; i < search_tags.size(); i++) {
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

	if (search_tags.size() > 0) {
		query.bindValue(":reference", QString("%%1%").arg(search_tags.join(" ")));
		for (int i = 0; i < search_tags.size(); i++) {
			query.bindValue(QString(":uid_").append(QString::number(i)), QString("%%1%").arg(search_tags.at(i)));
			query.bindValue(QString(":issue_date_").append(QString::number(i)), QString("%%1%").arg(search_tags.at(i)));
			query.bindValue(QString(":payable_").append(QString::number(i)), QString("%%1%").arg(search_tags.at(i)));
			query.bindValue(QString(":name_").append(QString::number(i)), QString("%%1%").arg(search_tags.at(i)));
			query.bindValue(QString(":email_").append(QString::number(i)), QString("%%1%").arg(search_tags.at(i)));
		}
	}
	query.exec();
	return query;
}

void SentBills::searchData() {
	QSettings settings;
	settings.setValue("sentbills/search_opposite", checkOpposite->isChecked());
	settings.setValue("sentbills/search_pending", checkOpen->isChecked());
	settings.setValue("sentbills/search_paid", checkPaid->isChecked());
	settings.setValue("sentbills/search_cancelled", checkCanceled->isChecked());
	settings.setValue("sentbills/search_contributed", checkContributed->isChecked());
	settings.setValue("sentbills/sincedate", sinceDate->date());
	settings.setValue("sentbills/maxdate", maxDate->date());

	QSqlQuery query = createQuery();
	tableModel->setQuery(query);
}

void SentBills::exportQifAssets() {
	exportInvoiceQifForm.fromDate->setDate( sinceDate->date() );
	exportInvoiceQifForm.toDate->setDate( maxDate->date() );
	invoiceQifDialog->show();
}

void SentBills::exportQifPayments() {
	exportPaymentQifForm.fromDate->setDate( sinceDate->date() );
	exportPaymentQifForm.toDate->setDate( maxDate->date() );
	paymentQifDialog->show();
}

void SentBills::doExportQifAssets() {
	QSettings settings;
	QHash<QString, QString * > list;
	QString qif_init("!Type:" + settings.value("qif/account_asset", "Oth A").toString());
	QString *qif;
#ifndef FIO
	float amountLimited = settings.value("invoice/amount_limited", 30.0).toFloat();
#endif

	QSqlQuery query(db);
	query.prepare("SELECT member_uid,reference,amount,issue_date,address_name,for_section,recommendations FROM pps_invoice WHERE issue_date >= :date1 AND issue_date <= :date2;");
	query.bindValue(":date1", exportInvoiceQifForm.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", exportInvoiceQifForm.toDate->date().toString("yyyy-MM-dd"));
	query.exec();

	while (query.next()) {
		QString member = query.value(0).toString();
		QString section;
#ifndef FIO
		section = query.value(5).toString();
		float amount = query.value(2).toFloat();
#else
		QStringList tmp, recom = query.value(6).toString().split(";");
		for (int i = 0; i < recom.size(); i++) {
			tmp = recom.at(i).split(":");
			if (tmp.size() == 2) {
				section = Section::getSectionName(tmp.at(0));
				section = section.isEmpty() ? tmp.at(0) : section;
#endif

		if (!list.contains(section)) {
			list.insert(section, new QString(qif_init));
		}
		qif = list.value(section);
		qif->append("\nD").append(query.value(3).toString());
#ifdef FIO
		qif->append("\nT").append( QString::number(tmp.at(1).toFloat()) );
		qif->append("\nL").append(settings.value("qif/income_default", "Membership Default %1").toString().arg(section));
#else
		qif->append("\nT").append(query.value(2).toString());
		if (amount > amountLimited) {
			qif->append("\nL").append(settings.value("qif/income_limited", "Membership Limited %1").toString().arg( query.value(5).toString() ));
		} else {
			qif->append("\nL").append(settings.value("qif/income_default", "Membership Default %1").toString().arg( query.value(5).toString() ));
		}
#endif
		qif->append("\nP").append(settings.value("qif/invoice_label", tr("Membership: %1 (%2)")).toString().arg( query.value(4).toString(), member ));
		qif->append("\nN").append(query.value(1).toString());
		qif->append("\nM").append(settings.value("qif/memo", tr("Member UID: %1")).toString().arg(member));
		qif->append("\n^\n");
		qif = NULL;
#ifdef FIO
			}
		}
#endif
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif)"));
	if (!fileName.isEmpty()) {
		QList<QString> keys = list.keys();
		for (int i = 0; i < keys.size(); i++) {
			QFile f( QString(fileName).replace(".qif", "_" + keys.at(i) + ".qif") );
			if (f.open(QFile::WriteOnly | QFile::Truncate)) {
				QTextStream out(&f);
				out << *(list.value(keys.at(i)));
				
				delete list.value(keys.at(i));
				list.remove(keys.at(i));
			}
		}
	}
	invoiceQifDialog->hide();
}

void SentBills::doExportQifPayments() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_cash", "Cash").toString());
#ifdef FIO
	FIOCalc *fio = new FIOCalc;
#endif

	QSqlQuery query(db);
	query.prepare("SELECT member_uid,reference,amount_paid,paid_date,address_name,for_section,recommendations FROM pps_invoice WHERE paid_date >= :date1 AND paid_date <= :date2;");
	query.bindValue(":date1", exportPaymentQifForm.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", exportPaymentQifForm.toDate->date().toString("yyyy-MM-dd"));
	query.exec();

	while (query.next()) {
		// Build the QIF
		QString member = query.value(0).toString();
		qif.append("\nD").append(query.value(3).toString());
		qif.append("\nT").append(query.value(2).toString());
		qif.append("\nP").append(settings.value("qif/payee_label", tr("Payment: %1 (%2)")).toString().arg( query.value(4).toString(), member ) );
		qif.append("\nN").append(query.value(1).toString());
		qif.append("\nM").append(settings.value("qif/memo", tr("Member UID: %1")).toString().arg(member));
#ifndef FIO
		qif.append("\nL").append(settings.value("qif/account_income", "Membership fee %1").toString().arg( query.value(5).toString() ));
#else
		QString section;
		fio->reset();

		// Add all invloved sections with their recommendations.
		QStringList tmp, recom = query.value(6).toString().split(";");
		for (int i = 0; i < recom.size(); i++) {
			tmp = recom.at(i).split(":");
			if (tmp.size() == 2) {
				section = Section::getSectionName(tmp.at(0));
				section = section.isEmpty() ? tmp.at(0) : section;
				fio->addSection( section, tmp.at(1).toFloat() );
			}
		}
		QHash<QString, float> result = fio->calculate(query.value(2).toFloat());

		// Create a split-booking if there are more sections than one involved.
		if (result.size() > 1) {
			QHash<QString, float>::const_iterator it = result.constBegin();
			while (it != result.constEnd()) {
				qif.append("\nS").append(settings.value("qif/account_income", "Membership fee %1").toString().arg( it.key() ));
				qif.append("\nE").append(settings.value("qif/memo", tr("Member UID: %1")).toString().arg(member));
				qif.append("\n$").append(QString::number(it.value()));
				it++;
			}
		} else {
			qif.append("\nL").append(settings.value("qif/account_income", "Membership fee %1").toString().arg( query.value(5).toString() ));
		}
#endif
		qif.append("\n^\n");
	}
#ifdef FIO
	delete fio;
#endif

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

void SentBills::exportCsvList() {
	csvExport.fromDate->setDate(sinceDate->date());
	csvExport.toDate->setDate(maxDate->date());
	csvExport.stateOpen->setChecked(checkOpen->isChecked());
	csvExport.statePaid->setChecked(checkPaid->isChecked());
	csvExport.stateCanceled->setChecked(checkCanceled->isChecked());
	csvExport.stateContributed->setChecked(checkContributed->isChecked());
	csvDialog->show();
}

void SentBills::doExportCsvList() {
	QSqlQuery query(db);
	QString sql("SELECT pps_invoice.member_uid, pps_invoice.reference, pps_invoice.amount, pps_invoice.address_prefix, pps_invoice.address_name,"
		"pps_invoice.address_street1, pps_invoice.address_street2, pps_invoice.address_city, ldap_persons.country"
		" FROM pps_invoice LEFT JOIN ldap_persons ON (pps_invoice.member_uid = ldap_persons.uid)"
		" WHERE pps_invoice.issue_date >= :date1 AND pps_invoice.issue_date <= :date2");
	if (csvExport.radioEmail->isChecked()) {
		sql.append(" AND (ldap_persons.notify_method=" + QString::number(PPSPerson::EMail) + " AND pps_invoice.address_email<>\"\")");
	} else if (csvExport.radioSnailMail->isChecked()) {
		sql.append(" AND (ldap_persons.notify_method=" + QString::number(PPSPerson::SnailMail) + " OR pps_invoice.address_email=\"\")");
	}
	if (csvExport.stateOpen->isChecked()) {
		sql.append(" AND pps_invoice.state=" + QString::number((int)Invoice::StateOpen));
	}
	query.prepare(sql);
	query.bindValue(":date1", csvExport.fromDate->date().toString("yyyy-MM-dd"));
	query.bindValue(":date2", csvExport.toDate->date().toString("yyyy-MM-dd"));
	if (!query.exec()) {
		qDebug() << query.lastError().text();
		qDebug() << query.lastQuery();
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save list as CSV"), "", tr("Comma separated file (*.csv);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << "\"member_uid\",\"reference\",\"amount\",\"prefix\",\"name\",\"address\",\"city\",\"country\"\n";

			while (query.next()) {
				out << query.value(0).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(1).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(2).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(3).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(4).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(5).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(7).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\",").prepend("\"");
				out << query.value(8).toString().replace('\n', "").replace('\r', "").replace('"',"\"\"").append("\"\n").prepend("\"");
			}
		}
	}
	csvDialog->close();
}

void SentBills::massChangeState(QAction *action) {
	QString state;
	switch (action->data().toInt()) {
		case Invoice::StateOpen:
			state = tr("Open");
			break;
		case Invoice::StateCanceled:
			state = tr("Canceled");
			break;
		case Invoice::StateContributed:
			state = tr("Contributed");
			break;
		case Invoice::StatePaid:
			state = tr("Paid");
			break;
		case Invoice::StateUnknown:
			state = tr("Unknown");
			break;
	}
	QSet<int> list = getSelectedRows();
	if (list.size() <= 0) {
		while (tableModel->canFetchMore()) {
			tableModel->fetchMore();
		}
	}

	int max = list.size() > 0 ? list.size() : tableModel->rowCount();
	QMessageBox::StandardButton btn = QMessageBox::warning(this, tr("Change invoice states"), tr("You are to change about %1 invoices to the State %2.\n"
		"If you are not sure, please cancel this action, you will not be able to undo this action!")
	.arg(max).arg(state), QMessageBox::Ok | QMessageBox::Cancel);

	if (btn == QMessageBox::Cancel) {
		return;
	}

	QSqlRecord record;
	QSqlQuery query(db);
	query.prepare("UPDATE pps_invoice SET state=:state WHERE reference=:reference;");
	query.bindValue(":state", action->data().toInt());

	// Check for select entries. If there are some, change only those. Otherwise change all visible.
	if (list.size() > 0) {
		foreach (const int &i, list) {
			record = tableModel->record(i);
			query.bindValue(":reference", record.value("reference").toString());
			query.exec();
		}
	} else {
		for (int i = 0; i < tableModel->rowCount(); i++) {
			record = tableModel->record(i);
			query.bindValue(":reference", record.value("reference").toString());
			query.exec();
		}
	}
	
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
	// TODO: Get the current Selected position and select this after searching data again
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
	QProgressDialog bar(this);
	QString fileName;
	Reminder reminder;
	PPSPerson pers;
	XmlPdf *pdf;
	QSet<int> rows = getSelectedRows();
	int max = rows.size();
	bar.setRange(0, max);
	bar.setCancelButtonText(tr("&Cancel"));
	bar.setWindowTitle(tr("Create Invoices"));
	bar.setWindowModality(Qt::WindowModal);

	bar.show();
	foreach (const int &i, rows) {
		uid = tableModel->record(i).value(0).toInt();
		reminder.loadLast(uid);
		reminder.setReminded(reminder.reminded() + 1);
		reminder.setLastReminded(QDate::currentDate());
		pers.load(uid);

		bar.setValue(i);
		bar.setLabelText(tr("Create Invoice for %1 (%2 of %3)").arg(reminder.addressName()).arg(i).arg(max) );

		if (!email || reminder.addressEmail().isEmpty() || (pers.isLoaded() && pers.notify() == PPSPerson::SnailMail)) {
			if (fileName.isEmpty()) {
				fileName = QFileDialog::getSaveFileName(this, tr("Save PDF-File"), "", tr("PDF (*.pdf)"));
				if (!fileName.isEmpty()) {
					fileName.replace(QRegExp("\\.pdf$"), "_%1.pdf");
				}
			}
			pdf = reminder.createPdf("reminder");
			pdf->print( QString(fileName).arg(reminder.memberUid()) );
		} else {
			pdf = reminder.createPdf("invoice");
			pdf->send(reminder.addressEmail());
		}
		reminder.save();
	}
	bar.setValue(max);
}

