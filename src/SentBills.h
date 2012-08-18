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

#ifndef SentBills_H
#define SentBills_H

#include "../ui_sentbills.h"
#include "../ui_adjustpaiddate.h"
#include "../ui_fromtodates.h"
#include "../ui_dateform.h"
#include "../ui_invoiceedit.h"
#include "data/Reminder.h"

#include <QWidget>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QTimerEvent>
#include <QPoint>
#include <QMenu>
#include <QSet>
#include <QAction>

class SentBills : public QWidget, private Ui::SentBillsForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	QAction *actionSendReminder;
	QAction *actionPrintReminder;
	QAction *actionShowEdit;
	
	QDialog *adjustMemberdateDialog;
	QDialog *invoiceQifDialog;
	QDialog *paymentQifDialog;
	QDialog *editDialog;
	Ui::adjustPaidDateForm adjustMembersDueDateForm;
	Ui::fromToDatesForm exportInvoiceQifForm;
	Ui::fromToDatesForm exportPaymentQifForm;
	Ui::editEditInvoice editInvoice;
	
	QSqlQuery createQuery();
	void createContextMenu();
	QSet<int> getSelectedRows();
	void createPdfReminds(bool email);

public:
	SentBills(QWidget *parent = 0);
	virtual ~SentBills();
	void loadData();

public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void doAdjustDates();
	void exportQifAssets();
	void doExportQifAssets();
	void exportQifPayments();
	void doExportQifPayments();
	void showTableContextMenu(const QPoint &point);
	void sendNewReminder();
	void printNewReminder();
	void showEditInvoiceForm();
	void doEditInvoice();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // SentBills_H