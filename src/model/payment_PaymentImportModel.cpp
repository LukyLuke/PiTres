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
	PaymentImportModel::PaymentImportModel(QObject *parent) : QAbstractListModel(parent) {
	}

	PaymentImportModel::~PaymentImportModel() {
		clear();
	}

	QVariant PaymentImportModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		if (l_items.size() <= index.row()) {
			return QVariant();
		}

		PaymentItem *entity = l_items.at(index.row());
		QVariant ret;
		ret.setValue<PaymentItem>(*entity);
		return ret;
	}

	void PaymentImportModel::insert(PaymentItem *entity) {
		beginResetModel();
		l_items.append(entity);
		endResetModel();
	}

	void PaymentImportModel::clear() {
		beginResetModel();
		while (!l_items.isEmpty()) {
			PaymentItem *entity = l_items.takeFirst();
			delete entity;
		}
		endResetModel();
	}


	qint32 PaymentImportModel::rowCount(const QModelIndex & /*parent*/) const {
		return l_items.size();
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
}
