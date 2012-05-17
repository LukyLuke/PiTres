/*
	<one line to give the program's name and a brief idea of what it does.>
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

#include "InvoiceWizard.h"
#include "data/Invoice.h"
#include "data/Person.h"
#include "helper/XmlPdf.h"

#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QListWidgetItem>
#include <QVariant>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QProgressDialog>
#include <QDebug>

InvoiceWizard::InvoiceWizard(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	newMemberDate->setDate(QDate::currentDate());
	memberUntil->setDate(QDate::currentDate());
	
	connect(memberUid, SIGNAL(returnPressed()), this, SLOT(insertMemberUid()));
	connect(btnUidInvoice, SIGNAL(clicked()), this, SLOT(invoiceMembers()));
	connect(btnInvoiceNew, SIGNAL(clicked()), this, SLOT(invoiceNewMembers()));
	connect(btnInvoiceUntil, SIGNAL(clicked()), this, SLOT(invoiceUntilDate()));
	connect(btnInvoiceAll, SIGNAL(clicked()), this, SLOT(invoiceAllMembers()));
	
	openDatabase();
}

void InvoiceWizard::openDatabase() {
	/*QSettings settings;
	QFileInfo dbfile(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	qDebug() << "Loading DB: " + dbfile.absoluteFilePath();
	
	if (!dbfile.absoluteDir().exists()) {
		dbfile.absoluteDir().mkpath(dbfile.absolutePath());
		qDebug() << "Path did not exists, created: " + dbfile.absolutePath();
	}
	
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbfile.absoluteFilePath());
	db.open();*/
}

InvoiceWizard::~InvoiceWizard() {
}

void InvoiceWizard::insertMemberUid() {
	PPSPerson pers;
	if (pers.load(memberUid->text().toInt())) { 
		memberUidList->addItem(memberUid->text());
		memberUid->clear();
	} else {
		QMessageBox::critical(this, tr("Unknown Member ID"), tr("The Member UID '%1' was not found, please remiport from LDAP if this UID should exist.").arg(memberUid->text()));
	}
}

void InvoiceWizard::invoiceMembers() {
	PPSPerson pers;
	Invoice invoice;
	XmlPdf *pdf;
	QString fileName;
	
	if (checkPrint->isChecked()) {
		fileName = getSaveFileName();
	}
	
	for (int i = 0; i < memberUidList->count(); i++) {
		QListWidgetItem *item = memberUidList->item(i);
		if (pers.load(item->text().toInt())) {
			invoice.create(&pers);
			if (checkPrint->isChecked()) {
				pdf = invoice.createPdf("reminder");
				pdf->print( QString(fileName).arg(invoice.memberUid()) );
			} else {
				pdf = invoice.createPdf("invoice");
				pdf->send(invoice.addressEmail());
			}
		}
	}
}

void InvoiceWizard::invoiceNewMembers() {
	QSqlQuery query(db);
	query.prepare("SELECT uid FROM ldap_persons WHERE joining>=?;");
	query.bindValue(0, newMemberDate->date().toString("yyyy-MM-dd"));
	query.exec();
	doCreateInvoices(&query);
}

void InvoiceWizard::invoiceUntilDate() {
	QSqlQuery query(db);
	query.prepare("SELECT uid FROM ldap_persons WHERE paid_due<? OR paid_due IS NULL or paid_due='';");
	query.bindValue(0, newMemberDate->date().toString("yyyy-MM-dd"));
	query.exec();
	doCreateInvoices(&query);
}

void InvoiceWizard::invoiceAllMembers() {
	QSqlQuery query(db);
	query.prepare("SELECT uid FROM ldap_persons;");
	query.exec();
	doCreateInvoices(&query);
}

void InvoiceWizard::doCreateInvoices(QSqlQuery *query) {
	PPSPerson pers;
	Invoice invoice;
	XmlPdf *pdf;
	QString fileName;
	
	if (checkPrintDate->isChecked()) {
		fileName = getSaveFileName();
	}
	
	int num = query->size();
	int cnt = 0;
	QProgressDialog bar(this);
	bar.setRange(0, num);
	bar.setCancelButtonText(tr("&Cancel"));
	bar.setWindowTitle(tr("Create Invoices"));
	bar.setWindowModality(Qt::WindowModal);
	bar.show();
	
	while (query->next()) {
		if (bar.wasCanceled()) {
			break;
		}
		bar.setValue(cnt++);
		
		if (pers.load(query->value(0).toInt())) {
			bar.setLabelText(tr("Create Invoice for %1 %2 (%3 of %4)").arg(pers.familyName()).arg(pers.givenName()).arg(cnt).arg(num) );
			
			invoice.create(&pers);
			if (checkPrintDate->isChecked()) {
				pdf = invoice.createPdf("reminder");
				pdf->print( QString(fileName).arg(invoice.memberUid()) );
			} else {
				pdf = invoice.createPdf("invoice");
				pdf->send(invoice.addressEmail());
			}
		}
	}
	bar.setValue(num);
}

QString InvoiceWizard::getSaveFileName() {
	QString fileName;
	while (true) {
		fileName = QFileDialog::getSaveFileName(this, tr("Save PDF-File"), "", tr("PDF (*.pdf)"));
		if (!fileName.isEmpty()) {
			fileName.replace(QRegExp("\\.pdf$"), "_%1.pdf");
			break;
		} else {
			int ret = QMessageBox::question(this, tr("Print instead of Save?"), tr("You have not given a Filename. Do you want to print the Invoices instead of save them as PDF?"), tr("Print Invoices"), tr("Save as PDF"));
			if (ret == 0) {
				break;
			}
		}
	}
	return fileName;
}
