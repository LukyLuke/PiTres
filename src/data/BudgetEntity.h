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

class BudgetEntity : public QObject {
Q_OBJECT
Q_PROPERTY(int id READ id WRITE setId)
Q_PROPERTY(QDate date READ date WRITE setDate)
Q_PROPERTY(QString description READ description WRITE setDescription)
Q_PROPERTY(float amount READ amount WRITE setAmount)
Q_PROPERTY(int section READ section WRITE setSection)

public:
	BudgetEntity(QObject *parent = 0);
	BudgetEntity(int id, QObject *parent = 0);
	virtual ~BudgetEntity();
	static void createTables();
	void load(int id);
	void save();
	
	// Setter
	void setId(int id);
	void setDate(QDate date);
	void setDescription(QString description);
	void setAmount(float amount);
	void setSection(int section);
	
	// Getter
	int id() { return i_id; };
	QDate date() { return d_date; };
	QString description() { return s_descr; };
	float amount() { return f_amount; };
	int section() { return i_section; };
	
private:
	QSqlDatabase db;
	bool _loaded;
	int i_id;
	QDate d_date;
	QString s_descr;
	float f_amount;
	int i_section;
	
	void clear();
};

#endif // BUDGETENTITY_H
