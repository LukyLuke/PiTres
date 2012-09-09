/*
	The BudgetView-Widget is for the Budget-Management and overview
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

#ifndef Budget_BudgetEntityDelegate_H
#define Budget_BudgetEntityDelegate_H

#include "data/BudgetEntity.h"

#include <QApplication>
#include <QSettings>
#include <QString>
#include <QList>
#include <QVariant>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QDebug>

namespace budget {
	class BudgetEntityDelegate : public QStyledItemDelegate {
	Q_OBJECT
	
	public:
		BudgetEntityDelegate(QObject * parent = 0);
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		
	private:
		mutable qint32 dateWidth;
	};
};
#endif // Budget_BudgetEntityDelegate_H