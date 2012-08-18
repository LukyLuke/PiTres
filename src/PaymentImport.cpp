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
#include "data/Invoice.h"

#include <QVariant>
#include <QDate>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

PaymentImport::PaymentImport(QWidget *parent) : QWidget(parent) {
	QSettings settings;
	reString = "^([\\d ]+) \\(([\\d-]+)\\): ([^(]+)\\((\\d+)\\) \\/ Section ([^\\s\\/]+) \\/ (\\d+) of (\\d+) paid$";
	listString = "%1 (%2): %3 (%4) / Section %5 / %6 of %7 paid";
	setupUi(this);
	dateEdit->setDate(QDate::currentDate());
	datePaidDue->setDate( QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-12-31").toString()), "yyyy-MM-dd") );
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(btnContinue, SIGNAL(clicked()), this, SLOT(continuePayment()));
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(addSelectedInvoice()));
	connect(listAvailable, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(invoiceSelected(QListWidgetItem*)));
}

PaymentImport::~PaymentImport() {
}

void PaymentImport::timerEvent(QTimerEvent *event) {
	killTimer(searchTimer);
	searchData();
}

void PaymentImport::searchData() {
	listAvailable->clear();
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
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
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
		listAvailable->addItem(listString.arg(
			query.value(fieldReference).toString(),
			query.value(fieldPayable).toDate().toString("yyyy-MM-dd"),
			query.value(fieldName).toString(),
			query.value(fieldMember).toString(),
			query.value(fieldSection).toString(),
			query.value(fieldAmountPaid).toString(),
			query.value(fieldAmount).toString()
		));
	}
}

void PaymentImport::searchDataTimeout(QString data) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void PaymentImport::invoiceSelected(QListWidgetItem *item) {
	QSettings settings;
	QRegExp re(reString);
	
	if (re.indexIn(item->text()) > -1) {
		editAmount->setValue(re.cap(7).toFloat() - re.cap(6).toFloat());
		datePaidDue->setDate(QDate::fromString(re.cap(2), "yyyy-MM-dd"));
		datePaidDue->setDate( QDate::fromString(datePaidDue->date().toString(settings.value("invoice/member_due_format", "yyyy-12-31").toString()), "yyyy-MM-dd") );
	 }
}

void PaymentImport::addSelectedInvoice() {
	if ((listAvailable->currentRow() > -1) && !editAmount->text().isEmpty()) {
		QString text = listAvailable->currentItem()->text();
		QRegExp re(reString);
		if (re.indexIn(text) > -1) {
			int row = tablePay->rowCount();
			tablePay->insertRow(row);
			tablePay->setItem(row, 0, new QTableWidgetItem( re.cap(1) ));
			tablePay->setItem(row, 1, new QTableWidgetItem(editAmount->text()));
			tablePay->setItem(row, 2, new QTableWidgetItem( re.cap(4) ));
			tablePay->setItem(row, 3, new QTableWidgetItem(dateEdit->date().toString("yyyy-MM-dd")));
			tablePay->setItem(row, 4, new QTableWidgetItem( re.cap(3) ));
			tablePay->setItem(row, 5, new QTableWidgetItem( re.cap(2) ));
			tablePay->setItem(row, 6, new QTableWidgetItem( re.cap(5) ));
			tablePay->setItem(row, 7, new QTableWidgetItem(datePaidDue->date().toString("yyyy-MM-dd")));
		} else {
			qDebug() << "Could not parse the TextLine, sorry...";
		}
	}
}

void PaymentImport::continuePayment() {
	QSettings settings;
	QString fileName;
	PPSPerson person;
	Invoice invoice;
	QDate valuta_date;
	//float amountLimited = settings.value("invoice/amount_limited", 30.0).toFloat();
	//float amountDefault = settings.value("invoice/amount_default", 60.0).toFloat();
	QString qif("!Type:" + settings.value("qif/account_bank", "Bank").toString());
	
	for (int i = 0; i < tablePay->rowCount(); i++) {
		QString ref = tablePay->item(i, 0)->text();
		QString amount = tablePay->item(i, 1)->text();
		QString member = tablePay->item(i, 2)->text();
		QString valuta = tablePay->item(i, 3)->text();
		QString name = tablePay->item(i, 4)->text();
		//QString section = tablePay->item(i, 6)->text();
		
		// Payment QIF
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP"+ settings.value("qif/payee_label", "Payment: ").toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", "Member UID: ").toString() + member);
		qif.append("\nL" + settings.value("qif/account_income", "Membership Fee").toString());
		qif.append("\n^\n");
		
		valuta_date = QDate::fromString(valuta, "yyyy-MM-dd");
		invoice.load(ref);
		if (invoice.pay(invoice.amountPaid() + amount.toFloat(), &valuta_date) && invoice.isPaid()) {
			person.load(member.toInt());
			person.setPaidDue(QDate::fromString(tablePay->item(i, 7)->text(), "yyyy-MM-dd"));
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

