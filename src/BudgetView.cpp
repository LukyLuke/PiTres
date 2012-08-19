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

#include "BudgetView.h"

#include <QDebug>

BudgetView::BudgetView(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
	connect(actionRemoveFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
	connect(actionChangeFolder, SIGNAL(triggered()), this, SLOT(editFolder()));
	
	connect(actionCreateEntry, SIGNAL(triggered()), this, SLOT(createEntry()));
	connect(actionRemoveEntry, SIGNAL(triggered()), this, SLOT(removeEntry()));
	connect(actionChangeEntry, SIGNAL(triggered()), this, SLOT(editEntry()));
	
	connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeClicked(QModelIndex)));
	
	treeModel = new budget::TreeModel(tr("Category"), tr("Description"));
	treeView->setModel(treeModel);
	
	tableModel = new QSqlQueryModel(tableView);
	tableModel->setHeaderData(0, Qt::Horizontal, tr("Date"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Description"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Balance"));
}

BudgetView::~BudgetView() { }

void BudgetView::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS budget_tree (entity_id INTEGER, parent_id INTEGER, name TEXT, description TEXT);");
	query.exec();
}

void BudgetView::treeClicked(const QModelIndex index) {
	budget::TreeItem *item = static_cast<budget::TreeItem *>(index.internalPointer());
	loadDetails(item->id());
	showSummary(item->id());
}

void BudgetView::loadDetails(int id) {
	
}

void BudgetView::showSummary(int id) {
	
}

void BudgetView::createFolder() {
	qDebug() << "Create Folder...";
}

void BudgetView::removeFolder() {
	qDebug() << "Remove Folder...";
}

void BudgetView::editFolder() {
	qDebug() << "Edit Folder...";
}

void BudgetView::createEntry() {
	qDebug() << "Create Entry...";
}

void BudgetView::removeEntry() {
	qDebug() << "Remove Entry...";
}

void BudgetView::editEntry() {
	qDebug() << "Edit Entry...";
}


/* Internal Classes in a separate namespace */
namespace budget {
	
	/* TreeView Itemts */
	TreeItem::TreeItem(const QVariant &data, const int id, TreeItem *parent) {
		parentItem = parent;
		itemData << data;
		entityId = id;
	}

	TreeItem::~TreeItem() {
		qDeleteAll(childItems);
	}
	
	void TreeItem::setComment(const QVariant &data) {
		itemData << data;
	}

	int TreeItem::id() const {
		return entityId;
	}

	void TreeItem::appendChild(TreeItem *child) {
		childItems.append(child);
	}

	TreeItem *TreeItem::child(int row) {
		return childItems.value(row);
	}

	int TreeItem::childCount() const {
		return childItems.count();
	}

	int TreeItem::columnCount() const {
		return itemData.count();
	}

	QVariant TreeItem::data(int column) const {
		return itemData.value(column);
	}

	int TreeItem::row() const {
		if (parentItem) {
			return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
		}
		return 0;
	}

	TreeItem* TreeItem::parent() {
		return parentItem;
	}

	/* The Model-Class implementation */
	TreeModel::TreeModel(const QString category, const QString comment, QObject *parent) : QAbstractItemModel(parent) {
		rootItem = new TreeItem(QString(category), 0);
		rootItem->setComment(comment);
		setupModelData(rootItem);
	}
	
	TreeModel::~TreeModel() {
		delete rootItem;
	}
	
	QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
		if (!hasIndex(row, column, parent)) {
			return QModelIndex();
		}
		
		TreeItem *parentItem;
		if (!parent.isValid()) {
			parentItem = rootItem;
		} else {
			parentItem = static_cast<TreeItem*>(parent.internalPointer());
		}
		
		TreeItem *childItem = parentItem->child(row);
		if (childItem) {
			return createIndex(row, column, childItem);
		}
		return QModelIndex();
	}
	
	QModelIndex TreeModel::parent(const QModelIndex &index) const {
		if (!index.isValid()) {
			return QModelIndex();
		}
		
		TreeItem *childItem = static_cast<TreeItem *>(index.internalPointer());
		TreeItem *parentItem = childItem->parent();
		
		if (parentItem == rootItem) {
			return QModelIndex();
		}
		return createIndex(parentItem->row(), 0, parentItem);
	}
	
	int TreeModel::rowCount(const QModelIndex &parent) const {
		TreeItem *parentItem;
		if (parent.column() > 0) {
			return 0;
		}
		
		if (!parent.isValid()) {
			parentItem = rootItem;
		} else {
			parentItem = static_cast<TreeItem *>(parent.internalPointer());
		}
		return parentItem->childCount();
	}
	
	int TreeModel::columnCount(const QModelIndex &parent) const {
		if (parent.isValid()) {
			return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
		}
		return rootItem->columnCount();
	}

	QVariant TreeModel::data(const QModelIndex &index, int role) const {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		
		TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
		return item->data(index.column());
	}
	
	Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
		if (index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			return rootItem->data(section);
		}
		return QVariant();
	}
	
	void TreeModel::setupModelData(TreeItem *parent) {
		QSqlQuery query(db);
		query.prepare("SELECT name,entity_id,description FROM budget_tree WHERE parent_id=? ORDER BY entity_id;");
		query.bindValue(0, parent->id());
		query.exec();
		while (query.next()) {
			TreeItem *child = new TreeItem( query.value(0).toString(), query.value(1).toInt(), parent);
			child->setComment(query.value(2).toString());
			setupModelData(child);
			parent->appendChild(child);
		}
	}

}
