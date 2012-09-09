/*
	An entry in a Budget-Folder
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

#ifndef BUDGETENTITY_H
#define BUDGETENTITY_H

#include <QObject>
#include <QDate>
#include <QString>
#include <QSqlDatabase>
#include <QList>
#include <QMetaType>
#include <QDebug>

class BudgetEntity : public QObject {
Q_OBJECT
Q_PROPERTY(qint32 id READ id WRITE setId)
Q_PROPERTY(QDate date READ date WRITE setDate)
Q_PROPERTY(QString description READ description WRITE setDescription)
Q_PROPERTY(qreal amount READ amount WRITE setAmount)
Q_PROPERTY(qint32 section READ section WRITE setSection)

public:
	BudgetEntity(QObject *parent = 0);
	BudgetEntity(qint32 id, QObject *parent = 0);
	BudgetEntity(const BudgetEntity &entity);
	virtual ~BudgetEntity();
	static void createTables();
	static QList<BudgetEntity *> *getEntities(qint32 section);
	static QList<BudgetEntity *> *getEntities(qint32 section, bool childs);
	void load(qint32 id);
	void save();
	void deleteItem();
	
	// Setter
	void setId(qint32 id);
	void setDate(QDate date);
	void setDescription(QString description);
	void setAmount(float amount);
	void setSection(qint32 section);
	
	// Getter
	qint32 id() const { return i_id; };
	QDate date() const { return d_date; };
	QString description() const { return s_descr; };
	qreal amount() const { return f_amount; };
	qint32 section() const { return i_section; };
	
private:
	QSqlDatabase db;
	bool _loaded;
	qint32 i_id;
	QDate d_date;
	QString s_descr;
	qreal f_amount;
	qint32 i_section;
	
	void clear();
	static QList<BudgetEntity *> getChildEntities(qint32 section);
};

Q_DECLARE_METATYPE(BudgetEntity);
QDebug operator<<(QDebug dbg, const BudgetEntity &entity);
QDebug operator<<(QDebug dbg, const BudgetEntity *entity);

#endif // BUDGETENTITY_H
