/*
	Import Payments and create QIF-Files for the Accounting
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

#include "PaymentImport.h"

PaymentImport::PaymentImport(QWidget *parent) : QWidget(parent) {
	QSettings settings;
	reString = "^([\\d ]+) \\(([\\d-]+)\\): ([^(]+)\\((\\d+)\\) \\/ Section ([^\\s\\/]+) \\/ (\\d+) of (\\d+) paid$";
	listString = "%1 (%2): %3 (%4) / Section %5 / %6 of %7 paid";
	setupUi(this);
	dateValuta->setDate(QDate::currentDate());
	datePaidDue->setDate( QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-02-15").toString()), "yyyy-MM-dd") );
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(btnContinue, SIGNAL(clicked()), this, SLOT(continuePayment()));
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(addSelectedInvoice()));
	connect(btnImport, SIGNAL(clicked()), this, SLOT(showPaymentsImportDialog()));

	// Helper EventFilter for returnPressed in SpinBox
	CatchKeyPress *amountKeyPress = new CatchKeyPress(this);
	editAmount->installEventFilter(amountKeyPress);
	connect(amountKeyPress, SIGNAL(returnPressed()), this, SLOT(addSelectedInvoice()));

	// List view displays all seacrh results and select the payed one.
	listAvailable->setModel(&listModel);
	listAvailable->setModelColumn(payment::PaymentItem::NUM_PROPERTIES);
	QItemSelectionModel *selectionModel = listAvailable->selectionModel();
	connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this, SLOT(invoiceRowChange(QModelIndex, QModelIndex)));

	// Table to display all payments getting imported.
	tablePay->setModel(&tableModel);
	tablePay->setColumnHidden(payment::PaymentItem::NUM_PROPERTIES, TRUE);

	// Prepare the FileImport dialog
	importDialog = new QDialog(this);
	import.setupUi(importDialog);
	import.importType->addItem(tr("ESR Record v3"), QVariant(0));
	import.importType->addItem(tr("ESR Record v4"), QVariant(1));
	import.paidDue->setDate( datePaidDue->date() );
	connect(import.actionImport, SIGNAL(triggered()), this, SLOT(importPaymentsFromDialog()));
	connect(import.actionParse, SIGNAL(triggered()), this, SLOT(parsePaymentsFromDialog()));
	connect(import.actionFile, SIGNAL(triggered()), this, SLOT(showPaymentsImportFilesDialog()));
	connect(btnImport, SIGNAL(clicked()), this, SLOT(showPaymentsImportDialog()));
}

PaymentImport::~PaymentImport() {
	delete importDialog;
}

void PaymentImport::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	searchData();
}

void PaymentImport::searchData() {
	listModel.clear();
	if (searchEdit->text().isEmpty()) {
		return;
	}

	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	QString search("reference LIKE :reference OR (");
	for (int i = 0; i < sl.size(); i++) {
		if (i > 0) {
			search.append(" AND ");
		}
		search.append("(");
		search.append( QString("member_uid = :uid_").append(QString::number(i)) );
		search.append( QString(" OR address_name LIKE :name_").append(QString::number(i)) );
		search.append(")");
	}
	search.append(")");

	QSqlQuery query(db);
	query.prepare(QString("SELECT member_uid,reference,payable_date,address_name,for_section,amount,amount_paid FROM pps_invoice WHERE %1;").arg(search));

	query.bindValue(QString(":reference"), QString("%%1%").arg(sl.join(" ")));
	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":uid_").append(QString::number(i)), sl.at(i));
		query.bindValue(QString(":name_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
	}

	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		return;
	}

	int fieldMember = query.record().indexOf("member_uid");
	int fieldReference = query.record().indexOf("reference");
	int fieldName = query.record().indexOf("address_name");
	int fieldSection = query.record().indexOf("for_section");
	int fieldPayable = query.record().indexOf("payable_date");
	int fieldAmount = query.record().indexOf("amount");
	int fieldAmountPaid = query.record().indexOf("amount_paid");

	while (query.next()) {
		payment::PaymentItem *entity = new payment::PaymentItem();
		entity->setReference( query.value(fieldReference).toString() );
		entity->setPayable( query.value(fieldPayable).toDate() );
		entity->setName( query.value(fieldName).toString() );
		entity->setMember( query.value(fieldMember).toInt() );
		entity->setSection( query.value(fieldSection).toString() );
		entity->setAmountPaidTotal( query.value(fieldAmountPaid).toFloat() );
		entity->setAmount( query.value(fieldAmount).toFloat() );
		listModel.insert(entity);
	}
}

void PaymentImport::searchDataTimeout(QString /*data*/) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void PaymentImport::invoiceRowChange(const QModelIndex &current, const QModelIndex & /*previous*/) {
	if (current.isValid()) {
		payment::PaymentItem entity = qvariant_cast< payment::PaymentItem >(listModel.index(current.row()).data(Qt::EditRole));
		invoiceSelected(entity);
	}
}

void PaymentImport::invoiceSelected(payment::PaymentItem item) {
	QSettings settings;
	QRegExp re(reString);
	QDate paidDue = QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-02-15").toString()), "yyyy-MM-dd");

	if (settings.value("invoice/member_due_next_year", TRUE).toBool()) {
		paidDue = paidDue.addYears(1);
	}
	if (item.valuta().isValid()) {
		dateValuta->setDate(item.valuta());
	}
	datePaidDue->setDate(paidDue);
	editAmount->setValue(item.amount() - item.amountPaidTotal());
}

void PaymentImport::addSelectedInvoice() {
	QModelIndex idx = listAvailable->selectionModel()->currentIndex();
	payment::PaymentItem entity = qvariant_cast< payment::PaymentItem >( listModel.data(idx, Qt::EditRole) );

	entity.setAmountPaid( editAmount->value() );
	entity.setValuta( dateValuta->date() );
	entity.setPaidDue( datePaidDue->date() );
	tableModel.insert(new payment::PaymentItem(entity));

	searchEdit->selectAll();
	searchEdit->setFocus();
}

void PaymentImport::continuePayment() {
	QSettings settings;
	QString fileName;
	PPSPerson person;
	Invoice invoice;
	QDate valuta;
	QString qif("!Type:" + settings.value("qif/account_bank", "Bank").toString());

	for (int i = 0; i < tableModel.rowCount(); i++) {
		payment::PaymentItem item = qvariant_cast< payment::PaymentItem >( tableModel.index(i).data(Qt::EditRole) );

		qif.append("\nD" + item.valuta().toString("yy-MM-dd") );
		qif.append("\nT" + QString::number(item.amountPaid()) );
		qif.append("\nP"+ settings.value("qif/payee_label", "Payment: %1 (%2)").toString().arg(item.name(), QString::number(item.member())) );
		qif.append("\nN" + item.reference());
		qif.append("\nM"+ settings.value("qif/memo", "Member UID: %1").toString().arg( QString::number(item.member()) ) );
		qif.append("\nL" + settings.value("qif/account_income", "Membership Fee %1").toString().arg(item.section()));
		qif.append("\n^\n");

		valuta = item.valuta();
		invoice.load(item.reference());
		if (invoice.pay( item.amountPaidTotal(), &valuta ) && invoice.isPaid()) {
			person.load( item.member() );
			person.setPaidDue( item.paidDue() );
			person.save();
		}
	}

	// Safe the QIF
	fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
}

void PaymentImport::showPaymentsImportDialog() {
	importDialog->show();
}

void PaymentImport::importPaymentsFromDialog() {
	QFile file(import.editFile->text());
	payment::PaymentItem *entity;
	Invoice invoice;

	if (file.open(QFile::ReadOnly)) {
		switch (import.importType->itemData( import.importType->currentIndex() ).toInt()) {
			case 0:
			{
				esr_record_3 record = parse_esr3(QString(file.readAll()));
				import.tableWidget->setRowCount(record.data.size());
				for (qint32 i = 0; i < record.data.size(); i++) {
					invoice.loadByReference(record.data.at(i).reference_number);
					if (invoice.memberUid() > 0) {
						entity = new payment::PaymentItem;
						entity->setReference( invoice.reference() );
						entity->setPayable( invoice.payableDue() );
						entity->setName( invoice.addressName() );
						entity->setMember( invoice.memberUid() );
						entity->setSection( invoice.forSection() );
						entity->setAmountPaidTotal( invoice.amountPaid() );
						entity->setAmount( invoice.amount() );

						entity->setAmountPaid( record.data.at(i).amount );
						entity->setValuta( record.data.at(i).valuta_date );
						entity->setPaidDue( import.paidDue->date() );

						tableModel.insert(entity);
						entity = NULL;
					} else {
						qDebug() << "Invoice not found: " << record.data.at(i).reference_number;
					}
				}
			}
				break;

			case 1:
			{
				esr_record_4 record = parse_esr4(QString(file.readAll()));
				import.tableWidget->setRowCount(record.data.size());
				for (qint32 i = 0; i < record.data.size(); i++) {
					invoice.loadByReference(record.data.at(i).reference_number);
					if (invoice.memberUid() > 0) {
						entity = new payment::PaymentItem;
						entity->setReference( invoice.reference() );
						entity->setPayable( invoice.payableDue() );
						entity->setName( invoice.addressName() );
						entity->setMember( invoice.memberUid() );
						entity->setSection( invoice.forSection() );
						entity->setAmountPaidTotal( invoice.amountPaid() );
						entity->setAmount( invoice.amount() );

						entity->setAmountPaid( record.data.at(i).amount );
						entity->setValuta( record.data.at(i).valuta_date );
						entity->setPaidDue( import.paidDue->date() );

						tableModel.insert(entity);
						entity = NULL;
					} else {
						qDebug() << "Invoice not found: " << record.data.at(i).reference_number;
					}
				}
			}
				break;

			default:
				QMessageBox::information(this, tr("Unknown file type selected"), tr("The selected Record-Type is currently not known."));
				break;
		}
	} else {
		QMessageBox::warning(this, tr("Cannot open selected File"), tr("The file you selected is not readable by your user.\nPlease correct the access rules or select an other file."), QMessageBox::Ok);
	}
	importDialog->hide();
}

void PaymentImport::showPaymentsImportFilesDialog() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Payments import file"), "", tr("ESR-File Postfinance (*.v11);;Plaintext, CSV (*.txt,*.csv)"));
	if (!fileName.isEmpty()) {
		import.editFile->setText(fileName);
		import.actionParse->trigger();
	}
}

void PaymentImport::parsePaymentsFromDialog() {
	QFile file(import.editFile->text());
	esr_record_3 record3;
	esr_record_4 record4;

	import.tableWidget->clear();
	import.tableWidget->setRowCount(0);
	import.tableWidget->setColumnCount(3);
	import.tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Valuta")));
	import.tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Amount")));
	import.tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Reference")));
	import.tableWidget->horizontalHeader()->setStretchLastSection(TRUE);

	if (file.open(QFile::ReadOnly)) {
		switch (import.importType->itemData( import.importType->currentIndex() ).toInt()) {
			case 0:
				record3 = parse_esr3(QString(file.readAll()));
				import.tableWidget->setRowCount(record3.data.size());
				for (qint32 i = 0; i < record3.data.size(); i++) {
					_addParsedImportLine( i, record3.data.at(i).valuta_date, record3.data.at(i).amount, record3.data.at(i).reference_number );
				}
				break;

			case 1:
				record4 = parse_esr4(QString(file.readAll()));
				import.tableWidget->setRowCount(record4.data.size());
				for (qint32 i = 0; i < record4.data.size(); i++) {
					_addParsedImportLine( i, record4.data.at(i).valuta_date, record4.data.at(i).amount, record4.data.at(i).reference_number );
				}
				break;

			default:
				QMessageBox::information(this, tr("Unknown file type selected"), tr("The selected Record-Type is currently not known."));
				break;
		}
	} else {
		QMessageBox::warning(this, tr("Cannot open selected File"), tr("The file you selected is not readable by your user.\nPlease correct the access rules or select an other file."), QMessageBox::Ok);
	}
}

void PaymentImport::_addParsedImportLine(qint32 row, QDate valuta, float amount, QString reference) {
	QTableWidgetItem *item;
	QColor color(255, 190, 160);
	Invoice invoice;
	invoice.loadByReference(reference);
	bool _valid = invoice.memberUid() > 0;

	// Valuta column
	item = new QTableWidgetItem( valuta.toString("yyyy-MM-dd") );
	if (!_valid) {
		item->setBackgroundColor(color);
	}
	import.tableWidget->setItem(row, 0, item);

	// Paid amount column.
	item = new QTableWidgetItem( tr("%1 sFr.").arg(amount, 0, 'f', 2) );
	if (!_valid) {
		item->setBackgroundColor(color);
	}
	import.tableWidget->setItem(row, 1, item);

	// Reference column.
	item = new QTableWidgetItem( reference );
	if (!_valid) {
		item->setBackgroundColor(color);
	}
	import.tableWidget->setItem(row, 2, item);
}