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

#include <QDebug>
#include <QSettings>

namespace budget {
	
	BudgetEntityModel::BudgetEntityModel(QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
	}
	
	BudgetEntityModel::BudgetEntityModel(QList<BudgetEntity *> *items, QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
		setData(items);
	}
	
	BudgetEntityModel::~BudgetEntityModel() {
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		delete entities;
	}
	
	void BudgetEntityModel::setData(QList<BudgetEntity *> *items) {
		beginResetModel();
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		entities = items;
		endResetModel();
	}
	
	qint32 BudgetEntityModel::rowCount(const QModelIndex &parent) const {
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
