/*
	The BudgetView-Widget is for the Budget-Management and overview
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

#ifndef BudgetView_H
#define BudgetView_H

#include "../ui_budget.h"

#include<QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class BudgetView : public QWidget, private Ui::BudgetForm {
Q_OBJECT

private:
	QSqlDatabase db;

public:
	BudgetView(QWidget *parent = 0);
	virtual ~BudgetView();

public slots:
	void createFolder();
	void removeFolder();
	void editFolder();
	void createEntry();
	void removeEntry();
	void editEntry();
	
};

#endif // BudgetView_H
