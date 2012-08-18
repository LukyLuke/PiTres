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

#ifndef PaymentImport_H
#define PaymentImport_H

#include "../ui_paymentimport.h"
#include "data/Person.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QTimerEvent>

// For QIF-Spec see http://www.respmech.com/mym2qifw/qif_new.htm

class PaymentImport : public QWidget, private Ui::PaymentImportForm {
Q_OBJECT
private:
	QSqlDatabase db;
	int searchTimer;
	QString listString;
	QString reString;

public:
	PaymentImport(QWidget *parent = 0);
	virtual ~PaymentImport();
	
public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void continuePayment();
	void addSelectedInvoice();
	void invoiceSelected(QListWidgetItem *item);
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // PaymentImport_H