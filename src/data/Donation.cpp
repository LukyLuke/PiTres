/*
	A single Donation-Entry
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


#include "Donation.h"

Donation::Donation(QObject *parent) : QObject(parent) {
	clear();
}

Donation::Donation(qint32 id, QObject *parent) : QObject(parent) {
	load(id);
}

Donation::Donation(const Donation &other) : QObject(other.parent()) {
	entityId = 0;
	i_donator = other.person();
	f_amount = other.amount();
	d_booked = other.booked();
	s_forSection = other.forSection();
	s_memo = other.memo();
	s_objective = other.objective();
	d_condtributed = other.contributed();
	s_source = other.source();
	s_addressPrefix = other.addressPrefix();
	s_addressCompany = other.addressCompany();
	s_addressName = other.addressName();
	s_addressStreet1 = other.addressStreet1();
	s_addressStreet2 = other.addressStreet2();
	s_addressCity = other.addressCity();
	s_addressCountry = other.addressCountry();
	s_addressEmail = other.addressEmail();
	s_addressPhone = other.addressPhone();
	s_addressMobile = other.addressMobile();
}

Donation::~Donation() {}

void Donation::clear() {
	_loaded = FALSE;
	entityId = 0;
	i_donator = 0;
	f_amount = 0;
	d_booked = QDate(1900, 0, 0);
	s_forSection = "";
	d_condtributed = QDate(1900, 0, 0);
	s_source = "";
	s_memo = "";
	s_objective = "";
	s_addressPrefix = "";
	s_addressCompany = "";
	s_addressName = "";
	s_addressStreet1 = "";
	s_addressStreet2 = "";
	s_addressCity = "";
	s_addressCountry = "";
	s_addressEmail = "";
	s_addressPhone = "";
	s_addressMobile = "";
}

void Donation::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.exec("CREATE TABLE IF NOT EXISTS donation_entities (entity_id INTEGER PRIMARY KEY AUTOINCREMENT, donator INTEGER, amount FLOAT, booked DATE,"
	           " section TEXT, contributed DATE, from_source TEXT, address_prefix TEXT, address_company TEXT, address_name TEXT, address_street1 TEXT,"
	           " address_street2 TEXT, address_city TEXT, address_country TEXT, address_email TEXT, address_phone TEXT, address_mobile TEXT,"
	           " booking_memo TEXT, objective TEXT);");
}

void Donation::deleteItem() {
	if (!_loaded) {
		return;
	}
	
	QSqlQuery query(db);
	query.prepare("DELETE FROM donation_entities WHERE entity_id=?;");
	query.bindValue(0, entityId);
	query.exec();
	clear();
}

void Donation::load(qint32 id) {
	clear();
	QSqlQuery query(db);
	query.prepare("SELECT * FROM donation_entities WHERE entity_id=?;");
	query.bindValue(0, id);
	query.exec();
	_loaded = query.first();
	
	if (_loaded) {
		QSqlRecord record = query.record();
		entityId = query.value(record.indexOf("entity_id")).toInt();
		setPerson(query.value(record.indexOf("donator")).toInt());
		setAmount(query.value(record.indexOf("amount")).toFloat());
		setBooked(query.value(record.indexOf("booked")).toDate());
		setForSection(query.value(record.indexOf("section")).toString());
		setMemo(query.value(record.indexOf("booking_memo")).toString());
		setObjective(query.value(record.indexOf("objective")).toString());
		setContributed(query.value(record.indexOf("contributed")).toDate());
		setSource(query.value(record.indexOf("from_source")).toString());
		setAddressPrefix(query.value(record.indexOf("address_prefix")).toString());
		setAddressCompany(query.value(record.indexOf("address_company")).toString());
		setAddressName(query.value(record.indexOf("address_name")).toString());
		setAddressStreet1(query.value(record.indexOf("address_street1")).toString());
		setAddressStreet2(query.value(record.indexOf("address_street2")).toString());
		setAddressCity(query.value(record.indexOf("address_city")).toString());
		setAddressCountry(query.value(record.indexOf("address_country")).toString());
		setAddressEmail(query.value(record.indexOf("address_email")).toString());
		setAddressPhone(query.value(record.indexOf("address_phone")).toString());
		setAddressMobile(query.value(record.indexOf("address_mobile")).toString());
	}
}

void Donation::save() {
	QSqlQuery query(db);
	if (_loaded) {
		query.prepare("UPDATE donation_entities SET donator=:donator,amount=:amount,booked=:booked,section=:section,contributed=:contributed,from_source=:from_source,"
		              "address_prefix=:address_prefix,address_company=:address_company,address_name=:address_name,address_street1=:address_street1,"
		              "address_street2=:address_street2,address_city=:address_city,address_country=:address_country,address_email=:address_email,"
		              "address_phone=:address_phone,address_mobile=:address_mobile,booking_memo:memo,objective:objective WHERE entity_id=:id;");
		query.bindValue(":id", entityId);
		
	} else {
		query.prepare("INSERT INTO donation_entities (donator,amount,booked,section,contributed,from_source,address_prefix,address_company,"
		              "address_name,address_street1,address_street2,address_city,address_country,address_email,address_phone,address_mobile,booking_memo,objective) "
		              "VALUES (:donator,:amount,:booked,:section,:contributed,:from_source,:address_prefix,:address_company,:address_name,"
		              ":adddress_street1,:address_street2,:address_city,:address_country,:address_email,:address_phone,:address_mobile,:memo,:objective);");
	}
	query.bindValue(":donator", i_donator);
	query.bindValue(":amount", f_amount);
	query.bindValue(":booked", d_booked);
	query.bindValue(":section", s_forSection);
	query.bindValue(":memo", s_memo);
	query.bindValue(":objective", s_objective);
	query.bindValue(":contributed", d_condtributed);
	query.bindValue(":from_source", s_source);
	query.bindValue(":address_prefix", s_addressPrefix);
	query.bindValue(":address_company", s_addressCompany);
	query.bindValue(":address_name", s_addressName);
	query.bindValue(":address_street1", s_addressStreet1);
	query.bindValue(":address_street2", s_addressStreet2);
	query.bindValue(":address_city", s_addressCity);
	query.bindValue(":address_country", s_addressCountry);
	query.bindValue(":address_email", s_addressEmail);
	query.bindValue(":address_phone", s_addressPhone);
	query.bindValue(":address_mobile", s_addressMobile);
	query.bindValue(":memo", s_memo);
	query.bindValue(":objective", s_objective);
	query.exec();
	
	// Get the ID if the entity was not loaded before
	if (!_loaded) {
		entityId = query.lastInsertId().toInt();
		_loaded = entityId > 0;
	}
}

void Donation::setPerson(qint32 person) {
	i_donator = person;
	emit personChanged(person);
}

void Donation::setAmount(float amount) {
	f_amount = amount;
	emit amountChanged(amount);
}

void Donation::setBooked(QDate date) {
	d_booked = date;
	emit bookedChanged(date);
}

void Donation::setForSection(QString section) {
	s_forSection = section;
	emit forSectionChanged(section);
}

void Donation::setContributed(QDate date) {
	d_condtributed = date;
	emit contributedChanged(date);
}

void Donation::setSource(QString source) {
	s_source = source;
	emit sourceChanged(source);
}

void Donation::setMemo(QString memo) {
	s_memo = memo;
	emit memoChanged(memo);
}

void Donation::setObjective(QString objective) {
	s_objective = objective.toLower();
	emit objectiveChanged(objective);
}

void Donation::setAddressPrefix(QString addressPrefix) {
	s_addressPrefix = addressPrefix;
	emit addressPrefixChanged(addressPrefix);
}

void Donation::setAddressCompany(QString addressCompany) {
	s_addressCompany = addressCompany;
	emit addressCompanyChanged(addressCompany);
}

void Donation::setAddressName(QString addressName) {
	s_addressName = addressName;
	emit addressNameChanged(addressName);
}

void Donation::setAddressStreet1(QString addressStreet1) {
	s_addressStreet1 = addressStreet1;
	emit addressStreet1Changed(addressStreet1);
}

void Donation::setAddressStreet2(QString addressStreet2) {
	s_addressStreet2 = addressStreet2;
	emit addressStreet2Changed(addressStreet2);
}

void Donation::setAddressCity(QString addressCity) {
	s_addressCity = addressCity;
	emit addressCityChanged(addressCity);
}

void Donation::setAddressCountry(QString addressCountry) {
	s_addressCountry = addressCountry;
	emit addressCountryChanged(addressCountry);
}

void Donation::setAddressEmail(QString addressEmail) {
	s_addressEmail = addressEmail;
	emit addressEmailChanged(addressEmail);
}

void Donation::setAddressPhone(QString addressPhone) {
	s_addressPhone = addressPhone;
	emit addressPhoneChanged(addressPhone);
}

void Donation::setAddressMobile(QString addressMobile) {
	s_addressMobile = addressMobile;
	emit addressMobileChanged(addressMobile);
}

