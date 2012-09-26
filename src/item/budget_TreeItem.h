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

#ifndef Budget_TreeItem_H
#define Budget_TreeItem_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSettings>
#include <QDebug>

namespace budget {
	/**
	* Items for the TreeView
	*/
	class TreeItem {
	public:
		TreeItem(TreeItem *parent = 0);
		virtual ~TreeItem();
		
		void appendChild(TreeItem *child);
		TreeItem *child(qint32 number);
		qint32 childCount() const;
		qint32 childNumber() const;
		qint32 columnCount() const;
		QVariant data(qint32 column) const;
		qint32 row() const;
		qint32 id() const;
		qint32 entityId() const;
		TreeItem *parent();
		void setParentId(qint32 parent);
		void setId(qint32 id);
		bool setData(const qint32 pos, const QVariant &data);
		bool insertChildren(qint32 position, qint32 count);
		bool removeChildren(qint32 position, qint32 count);
		void deleteItem();
		
	private:
		QSqlDatabase db;
		QList<QVariant> itemData;
		QList<TreeItem *> childItems;
		qint32 i_entityId;
		TreeItem *parentItem;
		void save();
	};
};
#endif // Budget_TreeItem_H