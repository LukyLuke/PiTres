/*
	The Budget-TreeModel is used in the TreeView of the Budget-View
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

#include "budget_TreeModel.h"

#include <QDebug>
#include <QSettings>

namespace budget {
	
	TreeModel::TreeModel(const QString number, const QString category, const QString comment, const QString amount, const QString type, QObject *parent) : QAbstractItemModel(parent) {
		rootItem = new TreeItem();
		rootItem->setData(0, number);
		rootItem->setData(1, category);
		rootItem->setData(2, comment);
		rootItem->setData(3, amount);
		rootItem->setData(4, type);
		
		setupModelData(rootItem);
	}
	
	TreeModel::~TreeModel() {
		delete rootItem;
	}
	
	QModelIndex TreeModel::index(qint32 row, qint32 column, const QModelIndex &parent) const {
		TreeItem *parentItem = getItem(parent);
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
		
		TreeItem *childItem = getItem(index);
		TreeItem *parentItem = childItem->parent();
		
		if (parentItem == rootItem) {
			return QModelIndex();
		}
		
		return createIndex(parentItem->childNumber(), 0, parentItem);
	}
	
	qint32 TreeModel::rowCount(const QModelIndex &parent) const {
		TreeItem *parentItem = getItem(parent);
		return parentItem->childCount();
	}
	
	qint32 TreeModel::columnCount(const QModelIndex & /*parent*/) const {
		return rootItem->columnCount();
	}
	
	QVariant TreeModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid()) {
			return QVariant();
		}
		if (role != Qt::DisplayRole && role != Qt::EditRole) {
			return QVariant();
		}
		return getItem(index)->data(index.column());
	}
	
	TreeItem *TreeModel::getItem(const QModelIndex &index) const {
		if (index.isValid()) {
			TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
			if (item) {
				return item;
			}
		}
		return rootItem;
	}
	
	Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
		if (!index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	}
	
	bool TreeModel::setHeaderData(qint32 section, Qt::Orientation orientation, const QVariant &value, qint32 role) {
		if (role != Qt::EditRole || orientation != Qt::Horizontal) {
			return false;
		}
		bool success = rootItem->setData(section, value);
		if (success) {
			emit headerDataChanged(orientation, section, section);
		}
		return success;
	}
	
	QVariant TreeModel::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			return rootItem->data(section);
		}
		return QVariant();
	}
	
	bool TreeModel::insertRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		TreeItem *parentItem = getItem(parent);
		bool success;
		
		beginInsertRows(parent, pos, pos + count - 1);
		success = parentItem->insertChildren(pos, count);
		endInsertRows();
		
		return success;
	}
	
	bool TreeModel::removeRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		TreeItem *parentItem = getItem(parent);
		bool success;
		
		beginRemoveRows(parent, pos, pos + count - 1);
		success = parentItem->removeChildren(pos, count);
		endRemoveRows();
		
		return success;
	}
	
	bool TreeModel::setData(const QModelIndex &index, const QVariant &value, qint32 role) {
		if (role != Qt::EditRole) {
			return false;
		}
		
		TreeItem *item = getItem(index);
		bool success = item->setData(index.column(), value);
		if (success) {
			emit dataChanged(index, index);
		}
		return success;
	}
	
	void TreeModel::setupModelData(TreeItem *parent) {
		QLocale locale;
		QSqlQuery query(db);
		query.prepare("SELECT entity_id,position,name,description,amount,type FROM budget_tree WHERE parent_id=? ORDER BY position ASC;");
		query.bindValue(0, parent->id());
		query.exec();
		
		while (query.next()) {
			TreeItem *child = new TreeItem(parent);
			child->setId(query.value(0).toInt());
			child->setData(0, query.value(1));
			child->setData(1, query.value(2));
			child->setData(2, query.value(3));
			child->setData(3, query.value(4).toFloat());
			child->setData(4, query.value(5).toInt());
			parent->appendChild(child);
			setupModelData(child);
		}
	}
	
}
