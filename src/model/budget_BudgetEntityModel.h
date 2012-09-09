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

#ifndef Budget_BudgetEntityModel_H
#define Budget_BudgetEntityModel_H

#include "data/BudgetEntity.h"

#include <QString>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QSettings>
#include <QDebug>

namespace budget {
	/**
	* The Model and ItemDelegate for the listView to show all entities
	*/
	class BudgetEntityModel : public QAbstractListModel {
	Q_OBJECT
		
	public:
		BudgetEntityModel(QObject *parent = 0);
		BudgetEntityModel(qint32 section, QList<BudgetEntity *> *entities, QObject *parent = 0);
		virtual ~BudgetEntityModel();
		void setData(qint32 section, QList<BudgetEntity *> *entities);
		void setSection(qint32 section) { i_section = section; };
		qint32 section() const { return i_section; };
		
		qint32 rowCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, qint32 role) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
		bool insertRows(qint32 row, qint32 count, const QModelIndex &parent = QModelIndex());
		Qt::ItemFlags flags(const QModelIndex & index) const;
		
	private:
		QList<BudgetEntity *> *entities;
		qint32 i_section;
	};
};
#endif // Budget_BudgetEntityModel_H