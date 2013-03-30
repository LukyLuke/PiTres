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
}

PaymentImport::~PaymentImport() {
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
		payment::PaymentItem item = qvariant_cast<payment::PaymentItem>( tableModel.index(i).data(Qt::EditRole) );
		
		qif.append("\nD" + item.valuta().toString("yy-MM-dd") );
		qif.append("\nT" + QString::number(item.amountPaid()) );
		qif.append("\nP"+ settings.value("qif/payee_label", "Payment: %1 (%2)").toString().arg(item.name(), item.member()) );
		qif.append("\nN" + item.reference());
		qif.append("\nM"+ settings.value("qif/memo", "Member UID: %1").toString().arg(item.member()) );
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
