/*
 Simple model used to preview invoices which will be created.             *
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

#include "invoice_InvoiceCreateModel.h"
namespace invoice {
	InvoiceCreateModel::InvoiceCreateModel(QObject *parent): QAbstractItemModel(parent) {
	}

	InvoiceCreateModel::~InvoiceCreateModel() {
		clear();
	}
	
	QVariant InvoiceCreateModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			switch (section) {
				case 0:  return tr("Member");
				case 1:  return tr("Name");
				case 2:  return tr("City");
				case 3:  return tr("Section");
				case 4:  return tr("Paid due");
				case 5:  return tr("Last paid");
				case 6:  return tr("Last invoice");
			}
		}
		return QVariant();
	}

	QVariant InvoiceCreateModel::data(const QModelIndex &index, int role) const {
		if (!index.isValid() || (role != Qt::DisplayRole)) {
			return QVariant();
		}
		if (l_items.size() <= index.row()) {
			return QVariant();
		}
		
		InvoiceCreateData entity = l_items.at(index.row());
		switch (index.column()) {
			case 0:  return QString::number(entity.uid);
			case 1:  return entity.address_name;
			case 2:  return entity.address_city;
			case 3:  return entity.section;
			case 4:  return entity.paidDue;
			case 5:  return entity.lastPaid;
			case 6:  return entity.lastInvoiceDate;
		}
		return QVariant();
	}

	void InvoiceCreateModel::insert(PPSPerson *person) {
		if (person->uid() <= 0) {
			return;
		}
		beginInsertRows(QModelIndex(), l_items.size(), l_items.size());

		Invoice *invoice = person->getInvoice();
		InvoiceCreateData entity;
		
		entity.uid = person->uid();
		entity.address_name = person->givenName() + " " + person->familyName();
		entity.address_city = person->postalCode() + " " + person->city();
		entity.section = person->section();
		entity.lastPaid = invoice->paidDate();
		entity.lastInvoiceDate = invoice->issueDate();
		entity.paidDue = person->paidDue();
		l_items.append(entity);
		endInsertRows();
	}

	void InvoiceCreateModel::clear() {
		beginRemoveRows(QModelIndex(), 0, l_items.size());
		l_items.clear();
		endRemoveRows();
	}

	int InvoiceCreateModel::rowCount(const QModelIndex & /*parent*/) const {
		return l_items.size();
	}

	int InvoiceCreateModel::columnCount(const QModelIndex & /*parent*/) const {
		return 7; // Number of attributes in InvoiceCreateData.
	}
	
	bool InvoiceCreateModel::insertRows(int pos, int count, const QModelIndex &parent) {
		beginInsertRows(parent, pos, pos + count - 1);
		for (qint32 i = 0; i < count; i++) {
			l_items.append(InvoiceCreateData());
		}
		endInsertRows();
		return true;
	}
	
	bool InvoiceCreateModel::removeRows(int pos, int count, const QModelIndex &parent) {
		if (l_items.size() < pos + count) {
			count = l_items.size() - pos;
		}
		
		beginRemoveRows(parent, pos, pos + count - 1);
		for (qint32 i = 0; i < count; i++) {
			l_items.takeAt(pos);
		}
		endRemoveRows();
		return true;
	}
	
	Qt::ItemFlags InvoiceCreateModel::flags(const QModelIndex &index) const {
		if (!index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	void InvoiceCreateModel::sort(int column, Qt::SortOrder order) {
		QAbstractItemModel::sort(column, order);
	}

	QModelIndex InvoiceCreateModel::index(int row, int column, const QModelIndex & /*parent*/ ) const {
		if (l_items.size() > row && row >= 0) {
			return createIndex(row, column, l_items.at(row).uid);
		}
		return QModelIndex();
	}

	QModelIndex InvoiceCreateModel::parent(const QModelIndex & /*child*/ ) const {
		return QModelIndex();
	}

}