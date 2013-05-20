/*
	Create new Invoices, based on Dates or on UIDs
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
	memberUntil->setDate(QDate(QDate::currentDate().year() + 1, 12, 31));

	connect(memberUid, SIGNAL(returnPressed()), this, SLOT(insertMemberUid()));
	connect(btnAddMember, SIGNAL(pressed()), this, SLOT(insertMemberUid()));
	connect(btnUidInvoice, SIGNAL(pressed()), this, SLOT(invoiceMembers()));

	connect(radioNew, SIGNAL(toggled(bool)), this, SLOT(updatePreviewTable()));
	connect(radioAll, SIGNAL(toggled(bool)), this, SLOT(updatePreviewTable()));
	connect(radioPaidDue, SIGNAL(toggled(bool)), this, SLOT(updatePreviewTable()));
	connect(radioReminder, SIGNAL(toggled(bool)), this, SLOT(updatePreviewTable()));

	connect(newMemberDate, SIGNAL(dateChanged(QDate)), this, SLOT(startSearchTimer()));
	connect(memberUntil, SIGNAL(dateChanged(QDate)), this, SLOT(startSearchTimer()));
	connect(reminderChooser, SIGNAL(valueChanged(int)), this, SLOT(startSearchTimer()));
	connect(sectionList, SIGNAL(currentRowChanged(int)), this, SLOT(updatePreviewTable()));

	connect(btnInvoice, SIGNAL(pressed()), this, SLOT(createInvoices()));

	previewTable->setModel(&_previewModel);
	previewTable->setItemDelegateForColumn(4, new DateDelegate("yyyy-MM-dd"));
	previewTable->setItemDelegateForColumn(5, new DateDelegate("yyyy-MM-dd"));
	previewTable->setItemDelegateForColumn(6, new DateDelegate("yyyy-MM-dd"));
	fillSectionList();
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

	QProgressDialog bar(this);
	int max = memberUidList->count();
	bar.setRange(0, max);
	bar.setCancelButtonText(tr("&Cancel"));
	bar.setWindowTitle(tr("Create Invoices"));
	bar.setWindowModality(Qt::WindowModal);
	bar.show();

	for (int i = 0; i < max; i++) {
		QListWidgetItem *item = memberUidList->item(i);
		bar.setValue(i);
		if (pers.load(item->text().toInt())) {
			if (bar.wasCanceled()) break;
			bar.setLabelText(tr("Create Invoice for %1 %2 (%3 of %4)").arg(pers.familyName()).arg(pers.givenName()).arg(i).arg(max) );
			invoice.create(&pers);

			// Send invoice by SnailMail and not EMail if "print" is checked, the user likes to become SnailsMails or he ahs no EMailaddress
			// The "invoice" template is the one without an invoiceSlip, the "reminder" has attached one normally
			if (checkPrint->isChecked() || invoice.addressEmail().isEmpty() || (noSnailMail->isChecked() && pers.isLoaded() && (pers.notify() == PPSPerson::SnailMail))) {
				if (fileName.isEmpty()) {
					fileName = getSaveFileName();
				}
				pdf = invoice.createPdf("reminder");
				pdf->print( fileName.arg(invoice.memberUid()) );
			} else {
				pdf = invoice.createPdf("invoice");
				pdf->send(invoice.addressEmail());
			}
		}
	}
	bar.setValue(max);
}

void InvoiceWizard::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	updatePreviewTable();
}

void InvoiceWizard::startSearchTimer() {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void InvoiceWizard::updatePreviewTable() {
	_previewModel.clear();
	_importType = radioNew->isChecked() ? INVOICE_IMPORTTYPE_NEW :
		radioAll->isChecked() ? INVOICE_IMPORTTYPE_ALL :
		radioPaidDue->isChecked() ? INVOICE_IMPORTTYPE_OUTSTANDING :
		radioReminder->isChecked() ? INVOICE_IMPORTTYPE_REMINDER :
		INVOICE_IMPORTTYPE_UNKNOWN;

	// Enable/Disable elements
	newMemberDate->setEnabled(_importType == INVOICE_IMPORTTYPE_NEW);
	memberUntil->setEnabled(_importType == INVOICE_IMPORTTYPE_OUTSTANDING);
	reminderChooser->setEnabled(_importType == INVOICE_IMPORTTYPE_REMINDER);

	// Prepare values for the query.
	QString section = sectionList->currentIndex().isValid() ? sectionList->currentItem()->text() : "";
	QDate paid = QDate::currentDate();
	paid.addMonths(0 - reminderChooser->value());
	if (section == tr("All")) {
		section = "";
	}

	// Create the query and fill the table model.
	QSqlQuery query(db);
	switch (_importType) {
		case INVOICE_IMPORTTYPE_ALL:
			query.prepare(QString("SELECT ldap_personsuid FROM ldap_persons"
			" WHERE ldap_personstype<=? %1 ORDER BY ldap_persons.uid;").arg(section.isEmpty() ? "" : "AND section=?"));
			query.bindValue(0, PPSPerson::Pirate);
			if (!section.isEmpty()) {
				query.bindValue(1, section);
			}
			break;

		case INVOICE_IMPORTTYPE_NEW:
			query.prepare(QString("SELECT ldap_persons.uid FROM ldap_persons"
			" WHERE ldap_persons.joining>=? AND ldap_persons.type<=? %1 ORDER BY ldap_persons.uid;").arg(section.isEmpty() ? "" : "AND ldap_persons.section=?"));
			query.bindValue(0, newMemberDate->date().toString("yyyy-MM-dd"));
			query.bindValue(1, PPSPerson::Pirate);
			if (!section.isEmpty()) {
				query.bindValue(2, section);
			}
			break;

		case INVOICE_IMPORTTYPE_OUTSTANDING:
			query.prepare(QString("SELECT ldap_persons.uid FROM ldap_persons"
			" WHERE ldap_persons.type<=? %1 ORDER BY ldap_persons.uid;").arg(section.isEmpty() ? "" : "AND ldap_persons.section=?"));
			query.bindValue(0, PPSPerson::Pirate);
			if (!section.isEmpty()) {
				query.bindValue(1, section);
			}
			break;

		case INVOICE_IMPORTTYPE_REMINDER:
			query.prepare(QString("SELECT DISTINCT ldap_persons.uid FROM ldap_persons LEFT JOIN pps_invoice ON (ldap_persons.uid = pps_invoice.member_uid)"
				" WHERE pps_invoice.state=? AND (pps_invoice.paid_date IS NULL OR pps_invoice.paid_date='') AND pps_invoice.issue_date<=?"
				" AND ldap_persons.type<=? %1 ORDER BY ldap_persons.uid;").arg(section.isEmpty() ? "" : "AND ldap_persons.section=?"));
			query.bindValue(0, Invoice::StateOpen);
			query.bindValue(1, paid.toString("yyyy-MM-dd"));
			query.bindValue(2, PPSPerson::Pirate);
			if (!section.isEmpty()) {
				query.bindValue(3, section);
			}
			break;

		default:
			// Clear table model and return.
			return;
	}

	if (query.exec()) {
		PPSPerson *person = new PPSPerson;
		_previewModel.beginInsert();
		while (query.next()) {
			if (person->load(query.value(0).toInt())) {
				// We need to check if the person has paid or not - not possible in SQL.
				if ((_importType == INVOICE_IMPORTTYPE_OUTSTANDING) && (person->paidDue() >= memberUntil->date())) {
					continue;
				}
				_previewModel.insert(person);
			}
		}
		_previewModel.endInsert();
		delete person;
	}
}

void InvoiceWizard::fillSectionList() {
	QList<QString> list;
	Section::getSectionList(&list);
	sectionList->addItem(new QListWidgetItem(tr("All")));
	for (qint32 i = 0; i < list.length(); i++) {
		sectionList->addItem(new QListWidgetItem(list.at(i)));
	}
}

void InvoiceWizard::createInvoices() {
	PPSPerson pers;
	Invoice invoice;
	XmlPdf *pdf;
	QString fileName;
	bool reminder = (_importType == INVOICE_IMPORTTYPE_OUTSTANDING) || (_importType == INVOICE_IMPORTTYPE_REMINDER);

	int num = _previewModel.rowCount();
	int cnt = 0;
	QProgressDialog bar(this);
	bar.setRange(0, num);
	bar.setCancelButtonText(tr("&Cancel"));
	bar.setWindowTitle(tr("Create Invoices"));
	bar.setWindowModality(Qt::WindowModal);
	bar.show();

	for (qint32 i = 0; i < num; i++) {
		if (bar.wasCanceled()) {
			break;
		}
		bar.setValue(cnt++);

		qint32 uid = _previewModel.index(i, 0).data(Qt::DisplayRole).toInt();
		if (pers.load(uid)) {
			if (bar.wasCanceled()) break;
			bar.setLabelText(tr("Create Invoice for %1 %2 (%3 of %4)").arg(pers.familyName()).arg(pers.givenName()).arg(cnt).arg(num) );
			if (reminder) {
				invoice.loadLast(uid);
				invoice.setReminded(invoice.reminded() + 1);
				invoice.setLastReminded(QDate::currentDate());
			} else {
				invoice.create(&pers);
			}

			// Send invoice by SnailMail and not EMail if "print" is checked, the user likes to become SnailsMails or he ahs no EMailaddress
			// The "invoice" template is the one without an invoiceSlip, the "reminder" has attached one normally
			if (checkPrintDate->isChecked() || invoice.addressEmail().isEmpty() || (pers.isLoaded() && pers.notify() == PPSPerson::SnailMail)) {
				if (fileName.isEmpty()) {
					fileName = getSaveFileName();
				}
				pdf = invoice.createPdf("reminder");
				pdf->print( fileName.arg(invoice.memberUid()) );
			} else {
				pdf = invoice.createPdf("invoice");
				pdf->send(invoice.addressEmail());
			}
		}
	}
	bar.setValue(num);
	updatePreviewTable();
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
