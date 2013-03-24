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

// Initialize static members
QHash<QString, QString> Section::m_namecache = QHash<QString, QString>();
QHash<QString, QString> Section::m_desccache = QHash<QString, QString>();

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
	s_invoiceLogo = "";
	m_invoiceLanguage.clear();
	s_email = "";
	_loaded = FALSE;
}

void Section::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.exec("CREATE TABLE IF NOT EXISTS pps_sections (name TEXT, parent TEXT, amount FLOAT, description TEXT,"
	           "bank_account TEXT, address TEXT, treasurer INTEGER, founded DATE, invoice_logo TEXT, email TEXT);");
	query.exec("CREATE TABLE IF NOT EXISTS pps_sections_texts (section TEXT, language TEXT, invoice_text TEXT);");
}

void Section::getNameParentHash(QHash<QString, QString> *hash) {
	if (hash) {
		QString section, parent;
		QSqlDatabase db;
		QSqlQuery query(db);
		query.exec("SELECT name,parent FROM pps_sections ORDER BY parent,name ASC;");
		while (query.next()) {
			section = query.value(0).toString();
			parent = query.value(1).toString();
			if (!section.isEmpty() && !parent.isEmpty()) {
				hash->insert( section, parent );
			}
		}
	}
}

void Section::getSectionList(QList<QString> *list) {
	if (list) {
		QString section;
		QSqlDatabase db;
		QSqlQuery query(db);
		query.exec("SELECT name FROM pps_sections ORDER BY name,parent ASC;");
		while (query.next()) {
			section = query.value(0).toString();
			if (!section.isEmpty()) {
				list->append(section);
			}
		}
	}
}

QString Section::getSectionName(const QString key) {
	if (!m_namecache.contains(key)) {
		QSqlDatabase db;
		QSqlQuery query(db);
		query.prepare("SELECT name FROM pps_sections WHERE name=? OR description=?;");
		query.bindValue(0, key);
		query.bindValue(1, key);
		
		if (query.exec() && query.first()) {
			m_namecache.insert( key, query.value(0).toString() );
		}
	}
	return m_namecache.value(key);
}

QString Section::getSectionDescription(const QString key) {
	if (!m_desccache.contains(key)) {
		QSqlDatabase db;
		QSqlQuery query(db);
		query.prepare("SELECT description FROM pps_sections WHERE name=? OR description=?;");
		query.bindValue(0, key);
		query.bindValue(1, key);

		if (query.exec() && query.first()) {
			m_desccache.insert( key, query.value(0).toString() );
		}
	}
	return m_desccache.value(key);
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
		s_invoiceLogo = query.value( record.indexOf("invoice_logo") ).toString();
		s_email = query.value( record.indexOf("email") ).toString();
		
		// Load section invoice texts
		query.prepare("SELECT language, invoice_text FROM pps_sections_texts WHERE section=?;");
		query.bindValue(0, name);
		query.exec();
		while (query.next()) {
			m_invoiceLanguage.insert( query.value(0).toString(), query.value(1).toString() );
		}
		
	} else {
		qWarning() << "Error while loading Section with name " << name << "; Error: " << query.lastError();
	}
}

Section *Section::parent() {
	if (loaded() && !s_parent.isEmpty()) {
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
		query.prepare("UPDATE pps_sections SET description=:description,amount=:amount,parent=:parent,address=:address"
		              ",bank_account=:account,treasurer=:treasurer,founded=:founded,invoice_text=:text,invoice_logo=:logo"
		              ",email=:email WHERE name=:name;");
	} else {
		query.prepare("INSERT INTO pps_sections (name,description,amount,parent,address,bank_account,treasurer,founded,invoice_logo,email)"
		              " VALUES (:name,:description,:amount,:parent,:address,:account,:treasurer,:founded,:logo,:email);");
	}
	query.bindValue(":name", s_name);
	query.bindValue(":description", s_description);
	query.bindValue(":amount", f_amount);
	query.bindValue(":parent", s_parent);
	query.bindValue(":address", s_address);
	query.bindValue(":account", s_account);
	query.bindValue(":treasurer", i_treasurer);
	query.bindValue(":founded", d_founded);
	query.bindValue(":logo", s_invoiceLogo);
	query.bindValue(":email", s_email);
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		_loaded = FALSE;
	} else {
		_loaded = TRUE;
		
		// Update/Insert Section-Invoice texts
		query.prepare("DELETE * FROM pps_sections_texts WHERE section=?;");
		query.bindValue(0, s_name);
		
		QMap<QString, QString>::const_iterator it;
		query.prepare("INSERT INTO pps_sections_texts (section,language,invoice_text) VALUES (?,?,?);");
		for (it = m_invoiceLanguage.constBegin(); it != m_invoiceLanguage.constEnd(); it++) {
			query.bindValue(0, s_name);
			query.bindValue(1, it.key());
			query.bindValue(2, it.value());
			query.exec();
		}
	}
}

bool Section::logoIsFile() {
	return !s_invoiceLogo.contains('\n') && !s_invoiceLogo.contains('\r') && QFile::exists(s_invoiceLogo);
}

QString Section::email() {
	// build a standardized address if no email is set in the database
	if (s_email.isEmpty()) {
		QSettings settings;
		bool first = TRUE;
		QString mail(settings.value("pdf/email_prepend", "info").toString().append("@"));
		Section *s = this, *p = parent();
		while (p != NULL) {
			mail.append(s->name()).append(".");
			
			// Don't cleanup the section itself ;)
			if (first) {
				first = FALSE;
			} else {
				delete s;
			}
			s = p;
			p = s->parent();
		}
		if (p) {
			delete p;
		}
		return mail.append(settings.value("pdf/email_append", "piratenpartei.ch").toString());
		
	}
	return s_email;
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

void Section::setInvoiceText(QString text, QString language) {
	m_invoiceLanguage.insert(language, text);
	emit invoiceTextChanged(text, language);
}

void Section::setInvoiceLogo(QString logo) {
	s_invoiceLogo = logo;
	emit invoiceLogoChanged(logo);
}

void Section::setEmail(QString email) {
	s_email = email;
	emit emailChanged(email);
}
