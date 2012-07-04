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

Section::Section(QObject parent) : QObject(parent) {
	clear();
}

Section::Section(const QString name, QObject parent) : QObject(parent) {
	clear();
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
	_loaded = FALSE;
}

void Section::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS pps_sections (name TEXT, parent TEXT, amount FLOAT, description TEXT,"
	              "bank_account TEXT, address TEXT, treasurer INTEGER);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
}

void Section::load(const QString name) {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("SELECT * FROM pps_sections WHERE name=?;");
	query.bindValue(0, name);
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	_loaded = query.first();
	if (_loaded) {
		QSqlRecord record = query.record();
		s_parent = query.value( record.indexOf("parent") ).toString();
		s_name = query.value( record.indexOf("name") ).toString();
		f_amount = query.value( record.indexOf("amount") ).toFloat();
		s_description = query.value( record.indexOf("description") ).toString();
		s_address = query.value( record.indexOf("address") ).toString();
		s_account = query.value( record.indexOf("account") ).toString();
		i_treasurer = query.value( record.indexOf("treasurer") ).toInt();
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
		QDatabaseConnection db;
		QSqlQuery query(db);
		query.prepare("SELECT name FROM pps_section WHERE parent=?;");
		query.bindValue(0, s_name);
		query.exec();
		QSqlRecord record = query.record();
		
		int n = record.indexOf("name");
		while(query.next()) {
			Section *section = new Section;
			section->load(query.valueOf(n).toString());
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
		query.prepare("UPDATE pps_section SET description=:description,amount=:amount,parent=:parent,address=:address,account=:account,treasurer=:treasurer"
		              " WHERE name=:name;");
	} else {
		query.prepare("INSERT INTO pps_section SET (name,description,amount,parent,address,account,treasurer)"
		              " VALUES (:name,:description,:amount,:parent,:address,:account,:treasurer);");
	}
	query.bindValue(":name", s_name);
	query.bindValue(":description", s_description);
	query.bindValue(":amount", f_amount);
	query.bindValue(":parent", s_parent);
	query.bindValue(":address", s_address);
	query.bindValue(":account", s_account);
	query.bindValue(":treasurer", i_treasurer);
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
		_loaded = FALSE;
	} else {
		_loaded = TRUE;
	}
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