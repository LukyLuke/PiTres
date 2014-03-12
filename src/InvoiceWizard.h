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

#ifndef INVOICEWIZARD_H
#define INVOICEWIZARD_H

#include "ui_invoicewizard.h"
#include "data/Section.h"
#include "delegate/DateDelegate.h"
#include "model/invoice_InvoiceCreateModel.h"
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
#include <QSqlError>
#include <QProgressDialog>
#include <QWidget>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimerEvent>
#include <QDebug>

class InvoiceWizard : public QWidget, private Ui::InvoiceForm {
Q_OBJECT
public:
	InvoiceWizard(QWidget *parent = 0);
	virtual ~InvoiceWizard();
	enum InvoiceStateType { INVOICE_STATE_TYPE_UNKNOWN, INVOICE_STATE_TYPE_NEW, INVOICE_STATE_TYPE_OUTSTANDING, INVOICE_STATE_TYPE_REMINDER, INVOICE_STATE_TYPE_ALL };

private:
	QSqlDatabase db;
	InvoiceStateType _invoiceState;
	invoice::InvoiceCreateModel _previewModel;
	int searchTimer;
	void fillSectionList();
	QString getSaveFileName();
	
private slots:
	void startSearchTimer();
	void insertMemberUid();
	void invoiceMembers();
	void updatePreviewTable();
	void createInvoices();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // INVOICEWIZARD_H
