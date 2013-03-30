/*
	Simple ListModel used on PaymentImport function to set/show data in a list view.
	Copyright (C) 2013  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

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


#ifndef Payment_PaymentImportModel_H
#define Payment_PaymentImportModel_H

#include "item/payment_PaymentItem.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>

namespace payment {
	class PaymentImportModel:public QAbstractItemModel {
	Q_OBJECT
	
	public:
		PaymentImportModel(QObject *parent = 0);
		virtual ~PaymentImportModel();

		qint32 rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, qint32 role = Qt::DisplayRole) const;
		void insert(PaymentItem *entity);
		void clear();
		
		bool insertRows(qint32 pos, qint32 count, const QModelIndex &parent = QModelIndex());
		bool removeRows(qint32 pos, qint32 count, const QModelIndex &parent = QModelIndex());
		Qt::ItemFlags flags(const QModelIndex &index) const;
		void sort(qint32 column, Qt::SortOrder order = Qt::AscendingOrder);

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		qint32 columnCount(const QModelIndex &parent = QModelIndex()) const;
		QModelIndex index(qint32 row, qint32 column = 0, const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &child) const;

	private:
		QList< PaymentItem * > l_items;
	};
};

#endif // Payment_PaymentImportModel_H
