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

#include "data/BudgetEntity.h"

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

namespace budget {
	class TreeItem;
	class TreeModel;
};

class BudgetView : public QWidget, private Ui::BudgetForm {
Q_OBJECT

private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	budget::TreeModel *treeModel;
	
	void loadDetails(int id);
	void showSummary(int id);

public:
	BudgetView(QWidget *parent = 0);
	virtual ~BudgetView();
	static void createTables();

public slots:
	void createFolder();
	void removeFolder();
	void editFolder();
	void createEntry();
	void removeEntry();
	void editEntry();
	void treeClicked(const QModelIndex index);
	
};


namespace budget {
	/* Items for the TreeView */
	class TreeItem {
	public:
		TreeItem(const QVariant &data, const int id, TreeItem *parent = 0);
		virtual ~TreeItem();
		
		void appendChild(TreeItem *child);
		TreeItem *child(int row);
		int childCount() const;
		int columnCount() const;
		QVariant data(int column) const;
		int row() const;
		int id() const;
		TreeItem *parent();
		void setComment(const QVariant &data);
		
	private:
		QList<TreeItem *> childItems;
		QList<QVariant> itemData;
		int entityId;
		TreeItem *parentItem;
	};

	/* The TreeModel for the BudgetView */
	class TreeModel : public QAbstractItemModel {
	Q_OBJECT

	public:
		TreeModel(const QString category, const QString comment, QObject *parent = 0);
		virtual ~TreeModel();
		
		QVariant data(const QModelIndex &index, int role) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		
	private:
		void setupModelData(TreeItem *parent);
		
		QSqlDatabase db;
		TreeItem *rootItem;
	};
};

#endif // BudgetView_H
