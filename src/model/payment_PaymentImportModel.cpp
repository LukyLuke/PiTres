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

#include "payment_PaymentImportModel.h"

namespace payment {
	PaymentImportModel::PaymentImportModel(QObject *parent) : QAbstractItemModel(parent) {
	}

	PaymentImportModel::~PaymentImportModel() {
		clear();
	}

	QVariant PaymentImportModel::headerData(int section, Qt::Orientation orientation, int role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			return PaymentItem::headerData(section);
		}
		return QVariant();
	}

	QVariant PaymentImportModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
			return QVariant();
		}
		if (l_items.size() <= index.row()) {
			return QVariant();
		}

		PaymentItem *entity = l_items.at(index.row());
		QVariant ret;

		// EditRole means we need all data, while DisplayRole is for showing a string.
		if (role == Qt::EditRole) {
			ret.setValue<PaymentItem>(*entity);
		} else {
			ret.setValue<QString>(entity->getData(index.column()));
		}
		return ret;
	}

	void PaymentImportModel::insert(PaymentItem *entity) {
		beginInsertRows(QModelIndex(), l_items.size(), l_items.size());
		l_items.append(entity);
		endInsertRows();
	}

	void PaymentImportModel::clear() {
		beginRemoveRows(QModelIndex(), 0, l_items.size());
		while (!l_items.isEmpty()) {
			PaymentItem *entity = l_items.takeFirst();
			delete entity;
		}
		endRemoveRows();
	}


	qint32 PaymentImportModel::rowCount(const QModelIndex & /*parent*/) const {
		return l_items.size();
	}

	qint32 PaymentImportModel::columnCount(const QModelIndex & /*parent*/) const {
		return PaymentItem::NUM_PROPERTIES + 1;
	}

	bool PaymentImportModel::insertRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		beginInsertRows(parent, pos, pos + count - 1);
		for (qint32 i = 0; i < count; i++) {
			l_items.append(new PaymentItem(this));
		}
		endInsertRows();
		return true;
	}

	bool PaymentImportModel::removeRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		if (l_items.size() < pos + count) {
			count = l_items.size() - pos;
		}

		beginRemoveRows(parent, pos, pos + count - 1);
		for (qint32 i = 0; i < count; i++) {
			PaymentItem *item = l_items.takeAt(pos);
			delete item;
		}
		endRemoveRows();
		return true;
	}

	Qt::ItemFlags PaymentImportModel::flags(const QModelIndex &index) const {
		if (!index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	void PaymentImportModel::sort(qint32 column, Qt::SortOrder order) {
		QAbstractItemModel::sort(column, order);
	}

	QModelIndex PaymentImportModel::index(qint32 row, qint32 column, const QModelIndex & /*parent*/) const {
		if (l_items.size() > row && row >= 0) {
			return createIndex(row, column, l_items.at(row));
		}
		return QModelIndex();
	}

	QModelIndex PaymentImportModel::parent(const QModelIndex & /*child*/) const {
		return QModelIndex();
	}
}
