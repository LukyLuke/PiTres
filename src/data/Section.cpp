/*
	A Section, a Sub-Department of the main pirate party
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

#include "Section.h"

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QDate>
#include <QList>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

Section::Section(QObject *parent) : QObject(parent) {
	clear();
}

Section::Section(const QString name, QObject *parent) : QObject(parent) {
	load(name);
}

Section::~Section() {}

void Section::clear() {
	s_parent = "";
	s_name = "";
	f_amount = 0;
	s_description = "";
	s_address = "";
	s_account = "";
	i_treasurer = 0;
	d_founded = QDate::currentDate();
	_loaded = FALSE;
}

void Section::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS pps_sections (name TEXT, parent TEXT, amount FLOAT, description TEXT,"
	              "bank_account TEXT, address TEXT, treasurer INTEGER, founded DATE);");
	query.exec();
}

void Section::load(const QString name) {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("SELECT * FROM pps_sections WHERE name=?;");
	query.bindValue(0, name);
	query.exec();
	
	clear();
	_loaded = query.first();
	if (_loaded) {
		QSqlRecord record = query.record();
		s_parent = query.value( record.indexOf("parent") ).toString();
		s_name = query.value( record.indexOf("name") ).toString();
		f_amount = query.value( record.indexOf("amount") ).toFloat();
		s_description = query.value( record.indexOf("description") ).toString();
		s_address = query.value( record.indexOf("address") ).toString();
		s_account = query.value( record.indexOf("bank_account") ).toString();
		i_treasurer = query.value( record.indexOf("treasurer") ).toInt();
		d_founded = query.value( record.indexOf("founded") ).toDate();
	} else {
		qWarning() << "Error while loading Section with name " << name << "; Error: " << query.lastError();
	}
}

Section *Section::parent() {
	if (loaded()) {
		return new Section(s_parent);
	}
	return NULL;
}

QList<Section *> Section::children() {
	QList<Section *> list;
	if (loaded()) {
		QSqlDatabase db;
		QSqlQuery query(db);
		query.prepare("SELECT name FROM pps_sections WHERE parent=?;");
		query.bindValue(0, s_name);
		query.exec();
		QSqlRecord record = query.record();
		
		int n = record.indexOf("name");
		while(query.next()) {
			Section *section = new Section;
			section->load(query.value(n).toString());
			list.append(section);
		}
	}
	return list;
}

void Section::save() {
	if (s_name.isEmpty()) return;
	
	QSqlDatabase db;
	QSqlQuery query(db);
	if (loaded()) {
		query.prepare("UPDATE pps_sections SET description=:description,amount=:amount,parent=:parent,address=:address,bank_account=:account,treasurer=:treasurer,founded=:founded"
		              " WHERE name=:name;");
	} else {
		query.prepare("INSERT INTO pps_sections (name,description,amount,parent,address,bank_account,treasurer,founded)"
		              " VALUES (:name,:description,:amount,:parent,:address,:account,:treasurer,:founded);");
	}
	query.bindValue(":name", s_name);
	query.bindValue(":description", s_description);
	query.bindValue(":amount", f_amount);
	query.bindValue(":parent", s_parent);
	query.bindValue(":address", s_address);
	query.bindValue(":account", s_account);
	query.bindValue(":treasurer", i_treasurer);
	query.bindValue(":founded", d_founded);
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
		_loaded = FALSE;
	} else {
		_loaded = TRUE;
	}
}

void Section::setParent(QString parent) {
	s_parent = parent;
	emit parentChanged(parent);
}

void Section::setName(QString name) {
	s_name = name;
	emit nameChanged(name);
}

void Section::setAmount(float amount) {
	f_amount = amount;
	emit amountChanged(amount);
}

void Section::setDescription(QString description) {
	s_description = description;
	emit descriptionChanged(description);
}

void Section::setAddress(QString address) {
	s_address = address;
	emit addressChanged(address);
}

void Section::setAccount(QString account) {
	s_account = account;
	emit accountChanged(account);
}

void Section::setTreasurer(int treasurer) {
	i_treasurer = treasurer;
	emit treasurerChanged(treasurer);
}

void Section::setFoundingDate(QDate date) {
	d_founded = date;
	emit foundedChanged(date);
}
