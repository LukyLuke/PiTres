/*
	Shows a Window with all open Payments and let pay one in cash incl. a receipt
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

#include "PaymentWizard.h"
#include "data/Invoice.h"
#include "data/Person.h"
#include "helper/XmlPdf.h"

#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QList>
#include <QVariant>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QDebug>

PaymentWizard::PaymentWizard(QWidget * parent) : QDialog(parent) {
	QSettings settings;
	setupUi(this);
	amountBox->setValue(0);
	btnPaySelected->setEnabled(false);
	
	connect(btnCreatePayment, SIGNAL(clicked()), this, SLOT(createCashPayment()));
	connect(btnPaySelected, SIGNAL(clicked()), this, SLOT(paySelectedInCash()));
	connect(tableWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(selectRow(QModelIndex)));
}

PaymentWizard::~PaymentWizard() {
}

void PaymentWizard::setPerson(int uid) {
	QSettings settings;
	QDate paidDue = QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-02-15").toString()), "yyyy-MM-dd");
	Invoice *inv;
	person.clear();
	person.load(uid);
	
	if (settings.value("invoice/member_due_next_year", TRUE).toBool()) {
		paidDue = paidDue.addYears(1);
	}
	
	setWindowTitle(tr("Unpaid invoices from %1 %2 (%3)").arg(person.familyName(), person.givenName(), person.nickname()));
	amountBox->setValue(person.contributionClass() == PPSPerson::ContributeStudent ? settings.value("invoice/amount_limited", 30).toFloat() : settings.value("invoice/amount_default", 60).toFloat());
	paidDate->setDate(QDate::currentDate());
	paidDueDate->setDate(paidDue);
	
	QList<Invoice *> list = person.getInvoices();
	for (int i = 0; i < list.size(); i++) {
		inv = list.at(i);
		if (inv->state() == Invoice::StateOpen) {
			tableWidget->insertRow(i);
			tableWidget->setItem(i, 0, new QTableWidgetItem(inv->issueDate().toString("yyyy-MM-dd")));
			tableWidget->setItem(i, 1, new QTableWidgetItem(inv->paidDate().toString("yyyy-MM-dd")));
			tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(inv->amount())));
			tableWidget->setItem(i, 3, new QTableWidgetItem(QString::number(inv->amountPaid())));
			tableWidget->setItem(i, 4, new QTableWidgetItem(QString::number(inv->state())));
			tableWidget->setItem(i, 5, new QTableWidgetItem(inv->reference()));
		}
		delete list.at(i);
	}
}

void PaymentWizard::selectRow(QModelIndex index) {
	if (index.isValid()) {
		amountBox->setValue(tableWidget->item(index.row(), 2)->text().toFloat());
		reference = tableWidget->item(index.row(), 5)->text();
		btnPaySelected->setEnabled(true);
	} else {
		btnPaySelected->setEnabled(false);
	}
}

void PaymentWizard::createCashPayment() {
	invoice.create(&person);
	doPayInvoice();
	close();
}

void PaymentWizard::paySelectedInCash() {
	invoice.loadByReference(reference);
	doPayInvoice();
	close();
}

void PaymentWizard::doPayInvoice() {
	invoice.pay(amountBox->value(), new QDate(paidDate->date()));
	
	person.setPaidDue(paidDueDate->date());
	person.save();
	
	XmlPdf *pdf;
	QString fileName = "";
	if (saveReceipt->isChecked()) {
		fileName = QFileDialog::getSaveFileName(this, tr("Save PDF-File"), "", tr("PDF (*.pdf)"));
		if (!fileName.isEmpty()) {
			fileName.replace(QRegExp("\\.pdf$"), QString("_%1.pdf").arg(invoice.memberUid()));
		}
	}
	pdf = invoice.createPdf("receipt");
	pdf->print(fileName);
}
