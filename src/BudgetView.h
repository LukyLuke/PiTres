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

#include "ui_budget.h"
#include "data/BudgetEntity.h"
#include "item/budget_TreeItem.h"
#include "model/budget_TreeModel.h"
#include "model/budget_BudgetEntityModel.h"
#include "delegate/budget_BudgetEntityDelegate.h"
#include "delegate/budget_TreeItemDelegate.h"

#include<QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSqlQueryModel>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

class BudgetView : public QWidget, private Ui::BudgetForm {
Q_OBJECT

private:
	QSqlDatabase db;
	QLocale locale;
	budget::TreeModel *treeModel;
	budget::BudgetEntityModel *listModel;
	
	void showSummary(QList<BudgetEntity *> *list);
	void doRemoveFolder();
	void doRemoveEntry();

public:
	BudgetView(QWidget *parent = 0);
	virtual ~BudgetView();
	static void createTables();
	static QList<qint32> getChildSections(qint32 section);
	static QList<qint32> getChildSections(qint32 section, bool childs);

public slots:
	void createFolder();
	void removeFolder();
	void createRoot();
	void createEntry();
	void removeEntry();
	void treeClicked(const QModelIndex index);
	void treeSelectionChanged();
	
};

#endif // BudgetView_H
