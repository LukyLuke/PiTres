/*
	The BudgetEntityModel is used for the ListView to show all Budget-Positions
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

#include "budget_BudgetEntityModel.h"

namespace budget {
	
	BudgetEntityModel::BudgetEntityModel(QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
		i_section = 0;
	}
	
	BudgetEntityModel::BudgetEntityModel(qint32 section, QList<BudgetEntity *> *items, QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
		setData(section, items);
	}
	
	BudgetEntityModel::~BudgetEntityModel() {
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		delete entities;
	}
	
	Qt::ItemFlags BudgetEntityModel::flags(const QModelIndex &index) const {
		if (!index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	}
	
	void BudgetEntityModel::setData(qint32 section, QList<BudgetEntity *> *items) {
		beginResetModel();
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		entities = items;
		i_section = section;
		endResetModel();
	}
	
	bool BudgetEntityModel::setData(const QModelIndex &index, const QVariant &value, int role) {
		if (role != Qt::EditRole) {
			return false;
		}
		
		BudgetEntity entity = qvariant_cast<BudgetEntity>(index.data());
		BudgetEntity val = value.value<BudgetEntity>();
		
		for (qint32 i = 0; i < entities->size(); ++i) {
			if (entities->at(i)->id() == entity.id()) {
				entities->at(i)->setAmount( val.amount() );
				entities->at(i)->setDate( val.date() );
				entities->at(i)->setDescription( val.description() );
				entities->at(i)->setSection( val.section() );
				entities->at(i)->save();
				return true;
			}
		}
		
		return false;
	}
	
	bool BudgetEntityModel::insertRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		beginInsertRows(parent, pos, pos + count - 1);
		for (qint32 i = 0; i < count; ++i) {
			BudgetEntity *item = new BudgetEntity();
			item->setDate(QDate::currentDate());
			item->setAmount(0);
			item->setDescription(tr("Description"));
			item->setSection(i_section);
			item->save();
			entities->append(item);
		}
		endInsertRows();
		
		return TRUE;
	}
	
	qint32 BudgetEntityModel::rowCount(const QModelIndex & /*parent*/) const {
		if (entities) {
			return entities->size();
		}
		return 0;
	}
	
	QVariant BudgetEntityModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		if (!entities || entities->size() <= index.row()) {
			return QVariant();
		}
		
		BudgetEntity *entity = entities->at(index.row());
		QVariant ret;
		ret.setValue<BudgetEntity>(*entity);
		return ret;
	}
	
}
