/*
	The Budget-TreeItem is used in the TreeView of the Budget-View
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

#include "budget_TreeItem.h"

namespace budget {
	
	TreeItem::TreeItem(TreeItem *parent) {
		parentItem = parent;
		i_entityId = 0;
		itemData << "0" << "" << "" << "0.0" << "0";
	}
	
	TreeItem::~TreeItem() {
		qDeleteAll(childItems);
	}
	
	bool TreeItem::setData(const qint32 pos, const QVariant &data) {
		if (pos >= 0) {
			itemData[pos] = data;
			save();
			return true;
		}
		return false;
	}
	
	qint32 TreeItem::id() const {
		return itemData[0].toInt();
	}
	
	qint32 TreeItem::entityId() const {
		return i_entityId;
	}
	
	void TreeItem::setId(qint32 id) {
		i_entityId = id;
	}
	
	void TreeItem::save() {
		if (parentItem) {
			QSqlQuery query(db);
			if (i_entityId <= 0) {
				query.prepare("INSERT INTO budget_tree (position,parent_id,name,description,amount,type) VALUES (:pos,:parent,:name,:description,:amount,:type);");
			} else {
				query.prepare("UPDATE budget_tree SET position=:pos,parent_id=:parent,description=:description,amount=:amount,name=:name,type=:type WHERE entity_id=:id;");
				query.bindValue(":id", i_entityId);
			}
			if (itemData[0].toInt() == parentItem->id()) {
				itemData[0] = itemData[0].toInt() + 1;
			}
			query.bindValue(":pos", itemData[0].toInt());
			query.bindValue(":parent", parentItem->id());
			query.bindValue(":name", itemData[1]);
			query.bindValue(":description", itemData[2]);
			query.bindValue(":amount", itemData[3].toReal());
			query.bindValue(":type", itemData[4].toInt());
			query.exec();
			
			if (query.lastError().type() != QSqlError::NoError) {
				qDebug() << query.lastQuery();
				qDebug() << query.lastError();
			}
			if (i_entityId <= 0) {
				setId(query.lastInsertId().toInt());
			}
		}
	}
	
	void TreeItem::deleteItem() {
		QSqlQuery query(db);
		
		while (!childItems.isEmpty()) {
			TreeItem *child = childItems.takeLast();
			child->deleteItem();
			delete child;
		}
		
		query.prepare("DELETE FROM budget_tree WHERE entity_id=?;");
		query.bindValue(0, i_entityId);
		query.exec();
		
		// delete Budget-Entities also
		query.prepare("DELETE FROM budget_entities WHERE section=?;");
		query.bindValue(0, i_entityId);
		query.exec();
	}
	
	void TreeItem::appendChild(TreeItem *child) {
		childItems.append(child);
	}
	
	bool TreeItem::insertChildren(qint32 position, qint32 count) {
		if (position < 0 || position > childItems.size()) {
			return false;
		}
		for (qint32 row = 0; row < count; ++row) {
			TreeItem *item = new TreeItem(this);
			childItems.insert(position, item);
		}
		return true;
	}
	
	bool TreeItem::removeChildren(qint32 position, qint32 count) {
		if (position < 0 || position + count > childItems.size()) {
			return false;
		}
		for (qint32 row = 0; row < count; ++row) {
			TreeItem *item = childItems.takeAt(position);
			item->deleteItem();
			delete item;
		}
		return true;
	}
	
	TreeItem *TreeItem::child(qint32 number) {
		return childItems.value(number);
	}
	
	qint32 TreeItem::childCount() const {
		return childItems.count();
	}
	
	qint32 TreeItem::childNumber() const {
		if (parentItem) {
			return parentItem->childItems.indexOf(const_cast<TreeItem *>(this));
		}
		return 0;
	}
	
	qint32 TreeItem::columnCount() const {
		return itemData.count();
	}
	
	QVariant TreeItem::data(qint32 column) const {
		return itemData.value(column);
	}
	
	qint32 TreeItem::row() const {
		if (parentItem) {
			return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
		}
		return 0;
	}
	
	TreeItem* TreeItem::parent() {
		return parentItem;
	}
	
}
