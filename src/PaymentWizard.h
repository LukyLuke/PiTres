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

#ifndef PAYMENTWIZARD_H
#define PAYMENTWIZARD_H

#include "ui_payment.h"
#include "data/Person.h"
#include "data/Invoice.h"

#include <QWidget>
#include <QDialog>
#include <QModelIndex>
#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

class PaymentWizard : public QDialog, private Ui::PaymentForm {
Q_OBJECT
public:
	PaymentWizard(QWidget * parent = 0);
	virtual ~PaymentWizard();
	void setPerson(int uid);
	
private:
	QSqlDatabase db;
	PPSPerson person;
	Invoice invoice;
	QString reference;
	void doPayInvoice();
	
public slots:
	void selectRow(QModelIndex index);
	void createCashPayment();
	void paySelectedInCash();
	
};

#endif // PAYMENTWIZARD_H
