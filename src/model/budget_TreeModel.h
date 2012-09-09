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

#ifndef Budget_TreeModel_H
#define Budget_TreeModel_H

#include "item/budget_TreeItem.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSettings>
#include <QDebug>

namespace budget {
	/**
	* The TreeModel for the BudgetView
	*/
	class TreeModel : public QAbstractItemModel {
	Q_OBJECT

	public:
		TreeModel(const QString number, const QString category, const QString comment, const QString amount, const QString type, QObject *parent = 0);
		virtual ~TreeModel();
		
		QVariant data(const QModelIndex &index, qint32 role) const;
		bool setData(const QModelIndex &index, const QVariant &value, qint32 role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;
		QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role = Qt::DisplayRole) const;
		bool setHeaderData(qint32 section, Qt::Orientation orientation, const QVariant &value, qint32 role = Qt::EditRole);
		QModelIndex index(qint32 row, qint32 column, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &index) const;
		qint32 rowCount(const QModelIndex &parent = QModelIndex()) const;
		qint32 columnCount(const QModelIndex &parent = QModelIndex()) const;
		bool insertRows(qint32 pos, qint32 count, const QModelIndex &parent = QModelIndex());
		bool removeRows(qint32 pos, qint32 count, const QModelIndex &parent = QModelIndex());
		
	private:
		QSqlDatabase db;
		TreeItem *rootItem;
		
		void setupModelData(TreeItem *parent);
		TreeItem *getItem(const QModelIndex &index) const;
	};
};
#endif // Budget_TreeModel_H