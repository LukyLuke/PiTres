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

#ifndef INVOICEWIZARD_H
#define INVOICEWIZARD_H

#include "ui_invoicewizard.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>

class InvoiceWizard : public QWidget, private Ui::InvoiceForm {
Q_OBJECT
public:
	InvoiceWizard(QWidget *parent = 0);
	virtual ~InvoiceWizard();

private:
	QSqlDatabase db;
	void doCreateInvoices(QSqlQuery *query);
	QString getSaveFileName();
	
public slots:
	void insertMemberUid();
	void invoiceMembers();
	void invoiceNewMembers();
	void invoiceUntilDate();
	void invoiceAllMembers();
};

#endif // INVOICEWIZARD_H
