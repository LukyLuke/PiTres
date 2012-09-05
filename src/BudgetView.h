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
#include <QList>
#include <QPainter>
#include <QStyledItemDelegate>

namespace budget {
	class TreeItem;
	class TreeModel;
	class BudgetEntityModel;
	class BudgetEntityDelegate;
};

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
	void createRootFolder();
	void removeFolder();
	void editFolder();
	void createEntry();
	void removeEntry();
	void editEntry();
	void treeClicked(const QModelIndex index);
	
};


namespace budget {
	/**
	* Items for the TreeView
	*/
	class TreeItem {
	public:
		TreeItem(TreeItem *parent = 0);
		virtual ~TreeItem();
		
		void appendChild(TreeItem *child);
		TreeItem *child(qint32 row);
		qint32 childCount() const;
		qint32 childNumber() const;
		qint32 columnCount() const;
		QVariant data(qint32 column) const;
		qint32 row() const;
		qint32 id() const;
		TreeItem *parent();
		void setId(qint32 id);
		bool setData(const qint32 pos, const QVariant &data);
		bool insertChildren(qint32 position, qint32 count);
		bool removeChildren(qint32 position, qint32 count);
		
	private:
		QList<TreeItem *> childItems;
		QList<QVariant> itemData;
		qint32 entityId;
		TreeItem *parentItem;
	};

	/**
	* The TreeModel for the BudgetView
	*/
	class TreeModel : public QAbstractItemModel {
	Q_OBJECT

	public:
		TreeModel(const QString number, const QString category, const QString comment, const QString amount, QObject *parent = 0);
		virtual ~TreeModel();
		
		QVariant data(const QModelIndex &index, qint32 role) const;
		bool setData(const QModelIndex &index, const QVariant &value, qint32 role);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role = Qt::DisplayRole) const;
		bool setHeaderData(qint32 section, Qt::Orientation orientation, const QVariant &value, qint32 role);
		QModelIndex index(qint32 row, qint32 column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		qint32 rowCount(const QModelIndex &parent = QModelIndex()) const;
		qint32 columnCount(const QModelIndex &parent = QModelIndex()) const;
		bool insertRows(qint32 pos, qint32 count, const QModelIndex &parent);
		bool removeRows(qint32 pos, qint32 count, const QModelIndex &parent = QModelIndex());
		
	private:
		QSqlDatabase db;
		TreeItem *rootItem;
		
		void setupModelData(TreeItem *parent);
		TreeItem *getItem(const QModelIndex &index) const;
	};
	
	/**
	* The Model and ItemDelegate for the listView to show all entities
	*/
	class BudgetEntityModel : public QAbstractListModel {
	Q_OBJECT
		
	public:
		BudgetEntityModel(QObject *parent = 0);
		BudgetEntityModel(QList<BudgetEntity *> *entities, QObject *parent = 0);
		virtual ~BudgetEntityModel();
		
		qint32 rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, qint32 role) const;
		void setData(QList<BudgetEntity *> *entities);
		
	private:
		QList<BudgetEntity *> *entities;
	};
	
	class BudgetEntityDelegate : public QStyledItemDelegate {
	Q_OBJECT
	
	public:
		BudgetEntityDelegate(QObject * parent = 0);
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		
	private:
		qint32 dateWidth;
	};
	
};

#endif // BudgetView_H
