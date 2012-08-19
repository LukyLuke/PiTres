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

#include "BudgetEntity.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

BudgetEntity::BudgetEntity(QObject *parent) : QObject(parent) {
	clear();
}

BudgetEntity::BudgetEntity(int id, QObject *parent) : QObject(parent) {
	load(id);
}

BudgetEntity::~BudgetEntity() {}

void BudgetEntity::clear() {
	_loaded = false;
	i_id = 0;
	i_section = 0;
	d_date = QDate(1900, 0, 0);
	s_descr = "";
	f_amount = 0.0;
}

void BudgetEntity::save() {
	QSqlQuery query(db);
	if (_loaded) {
		query.prepare("UPDATE budget_entities SET section=:section,valuta=:date,description=:desc,amount=:amount WHERE entity_id=:id;");
		query.bindValue(":id", i_id);
	} else {
		query.prepare("INSERT INTO budget_entities (section,valuta,description,amount) VALUES (:section,:date,:descr,:amount);");
	}
	query.bindValue(":section", i_section);
	query.bindValue(":date", d_date.toString("yyyy-MM-dd"));
	query.bindValue(":descr", s_descr);
	query.bindValue(":amount", f_amount);
	query.exec();
	
	// Get the EntityId
	if (!_loaded) {
		setId(query.lastInsertId().toInt());
		_loaded = true;
	}
}

void BudgetEntity::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS budget_entities (entity_id INTEGER PRIMARY KEY AUTOINCREMENT, section INTEGER, valuta DATE, description TEXT, amount FLOAT);");
	query.exec();
}

void BudgetEntity::load(int id) {
	clear();
	QSqlQuery query(db);
	query.prepare("SELECT * FROM budget_entities WHERE entity_id=?;");
	query.bindValue(0, id);
	query.exec();
	_loaded = query.first();
	
	if (_loaded) {
		QSqlRecord record = query.record();
		setId(query.value(record.indexOf("entity_id")).toInt());
		setSection(query.value(record.indexOf("section")).toInt());
		setDate(query.value(record.indexOf("valuta")).toDate());
		setDescription(query.value(record.indexOf("description")).toString());
		setAmount(query.value(record.indexOf("amount")).toFloat());
	}
}

void BudgetEntity::setId(int id) {
	i_id = id;
}

void BudgetEntity::setDate(QDate date) {
	d_date = date;
}

void BudgetEntity::setDescription(QString description) {
	s_descr = description;
}

void BudgetEntity::setAmount(float amount) {
	f_amount = amount;
}

void BudgetEntity::setSection(int section) {
	i_section = section;
}
