/*
 Simple model used to preview invoices which will be created.
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


#ifndef Invoice_InvoiceCreateModel_H
#define Invoice_InvoiceCreateModel_H

#include "data/Person.h"
#include "data/Invoice.h"

#include <QString>
#include <QList>
#include <QModelIndex>
#include <QDate>

namespace invoice {
	class InvoiceCreateModel : public QAbstractItemModel {

	public:
		InvoiceCreateModel(QObject *parent = 0);
		virtual ~InvoiceCreateModel();

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		void insert(PPSPerson *person);
		void clear();

		bool insertRows(int pos, int count, const QModelIndex &parent = QModelIndex());
		bool removeRows(int pos, int count, const QModelIndex &parent = QModelIndex());
		Qt::ItemFlags flags(const QModelIndex &index) const;
		void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QModelIndex parent(const QModelIndex &child) const;
		QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	private:
		struct InvoiceCreateData {
			qint32 uid;
			QString address_name;
			QString address_city;
			QString section;
		};
		QList<InvoiceCreateData> l_items;
	};
};
#endif // Invoice_InvoiceCreateModel_H
