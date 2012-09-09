/*
	Show a List with all Users - Payments can be executed (PaymentWizard)
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

#ifndef Userlist_H
#define Userlist_H

#include "ui_userlist.h"
#include "ui_dateform.h"

#include "data/Person.h"
#include "data/Invoice.h"
#include "PaymentWizard.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QSizePolicy>
#include <QTableView>
#include <QModelIndex>
#include <QModelIndexList>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QSqlRecord>
#include <QFileDialog>
#include <QRegExp>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSet>
#include <QDialog>
#include <QTimerEvent>
#include <QDebug>

class Userlist : public QWidget, private Ui::UserlistForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	QAction *actionManualPayment;
	QAction *actionEditDueDate;
	
	QDialog *editDateDialog;
	Ui::dateForm dateForm;
	
	void loadSections();
	QSqlQuery createQuery();
	void createContextMenu();
	int getFirstSelectedUid();

public:
	Userlist(QWidget *parent = 0);
	virtual ~Userlist();
	void loadData();

public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void filterSection(QString section);
	void showDetails(int index);
	void showDetails(QModelIndex index);
	void showTableContextMenu(const QPoint &point);
	void exportData();
	void payMemberfeeCash();
	void showMemberDueAdjust();
	void adjustMemberDueDate();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // Userlist_H