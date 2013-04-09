/*
	A Person
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

#include "Person.h"
#include "../helper/Formatter.h"

#include <QObject>
#include <QString>
#include <QRegExp>
#include <QVariant>
#include <QDate>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

PPSPerson::PPSPerson(QObject *parent) : QObject(parent) {
	clear();
}

PPSPerson::PPSPerson(const PPSPerson &other) : QObject(other.parent()) {
	// TODO: Implement
}

PPSPerson::~PPSPerson() {}

void PPSPerson::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons (uid INTEGER, contribution INTEGER, nickname TEXT, gender TEXT, familyname TEXT, "
	              "givenname TEXT, address TEXT, plz TEXT, city TEXT, country TEXT, state TEXT, birthday DATE, language TEXT, joining DATE, section TEXT, "
	              "type INTEGER, notify_method INTEGER);");
	query.exec();

	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_email (uid INTEGER, type TEXT, mail TEXT);");
	query.exec();

	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_telephone (uid INTEGER, type TEXT, number TEXT);");
	query.exec();

	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_mobile (uid INTEGER, type TEXT, number TEXT);");
	query.exec();

	// Persistent matching table between PaidUntilDates and MemberUIDs
	// insert into ldap_persons_dates select distinct member_uid,"2012-12-31" from pps_invoice where state = 2 and paid_date > "2011-12-31"
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_dates (uid INTEGER, paid_due DATE);");
	query.exec();
}

void PPSPerson::emptyTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("DELETE FROM ldap_persons;");
	query.exec();

	query.prepare("DELETE FROM ldap_persons_email;");
	query.exec();

	query.prepare("DELETE FROM ldap_persons_telephone;");
	query.exec();

	query.prepare("DELETE FROM ldap_persons_mobile;");
	query.exec();

	query.prepare("VACUUM;");
	query.exec();
}

void PPSPerson::resetMembertypes() {
	QSqlDatabase db;
	QSqlQuery save(db);
	QSqlQuery query(db);
	query.prepare("SELECT ldap_persons.uid,MAX(ldap_persons_dates.paid_due) AS paid_due,ldap_persons.type FROM ldap_persons"
	              " LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) GROUP BY ldap_persons_dates.uid;");
	save.prepare("UPDATE ldap_persons SET type=:type WHERE uid=:uid;");

	QDate now = QDate::currentDate();
	int type;
	query.exec();
	while (query.next()) {
		type = query.value(1).toDate() > now ? Pirate : Sympathizer;
		if (type != query.value(2).toInt()) {
			save.bindValue(":uid", query.value(0).toString());
			save.bindValue(":type", type);
			save.exec();
		}
	}
}

void PPSPerson::save() {
	int i;
	QSqlQuery query(db);

	// Persons are Readonly, expect when they are loaded from LDAP
	if (!is_loaded) {
		query.prepare("INSERT INTO ldap_persons (uid, contribution, nickname, gender, familyname, givenname, address, plz, city, country, state, birthday, language, joining, section, type, notify_method) "
		              "VALUES (:uid, :contcls, :nickname, :gender, :familyname, :givenname, :address, :plz, :city, :country, :state, :birthday, :language, :joining, :section, :type, :notify);");
		query.bindValue(":uid", i_uid);
		query.bindValue(":section", s_section);
		query.bindValue(":contcls", (int)m_contributionClass);
		query.bindValue(":nickname", s_nickname);
		query.bindValue(":gender", (int)m_gender);
		query.bindValue(":familyname", s_familyName);
		query.bindValue(":givenname", s_givenName);
		query.bindValue(":address", s_street);
		query.bindValue(":plz", s_postalCode);
		query.bindValue(":city", s_city);
		query.bindValue(":country", s_country);
		query.bindValue(":state", s_state);
		query.bindValue(":birthday", d_birthdate);
		query.bindValue(":joining", d_joining);
		query.bindValue(":language", (int)m_language);
		query.bindValue(":type", (int)m_type);
		query.bindValue(":notify", (int)m_notify);
		query.exec();

		// Insert all telephones, mobiles and email addresses
		if (!l_telephone.empty()) {
			query.prepare("INSERT INTO ldap_persons_telephone (uid, type, number) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_telephone.size(); i++) {
				query.bindValue(":value", l_telephone.at(i));
				query.exec();
			}
		}

		if (!l_mobile.empty()) {
			query.prepare("INSERT INTO ldap_persons_mobile (uid, type, number) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_mobile.size(); i++) {
				query.bindValue(":value", l_mobile.at(i));
				query.exec();
			}
		}

		if (!l_email.empty()) {
			query.prepare("INSERT INTO ldap_persons_email (uid, type, mail) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_email.size(); i++) {
				query.bindValue(":value", l_email.at(i));
				query.exec();
			}
		}
	}

	if (!l_paidDue.isEmpty()) {
		// Remove all dates
		query.prepare("DELETE FROM ldap_persons_dates WHERE uid=:uid;");
		query.bindValue(":uid", i_uid);
		query.exec();

		// Insert all paid dates
		query.prepare("INSERT INTO ldap_persons_dates (uid,paid_due) VALUES (:uid,:due);");
		query.bindValue(":uid", i_uid);
		for (i = 0; i < l_paidDue.size(); i++) {
			query.bindValue(":due", l_paidDue.at(i));
			query.exec();
		}
	}
}

void PPSPerson::clear() {
	is_loaded = FALSE;
	i_uid = 0;
	s_section = "";
	m_contributionClass = ContributeFull;
	s_nickname = "";
	m_gender = GenderUnknown;
	s_familyName = "";
	s_givenName = "";
	s_street = "";
	s_postalCode = "";
	s_city = "";
	s_country = "";
	s_state = "";
	l_telephone.clear();
	l_mobile.clear();
	l_email.clear();
	d_birthdate = QDate(1970, 1, 1);
	d_joining = QDate::currentDate();
	m_language = EN;
	l_paidDue.clear();
	m_type = Pirate;
	m_notify = EMail;
	_invoice.clear();
	l_paidDue.append(QDate(1970, 1, 1));
}

bool PPSPerson::load(int uid) {
	clear();
	i_uid = uid;

	QSqlQuery query(db);
	query.prepare("SELECT * FROM ldap_persons WHERE uid=?;");
	query.bindValue(0, uid);
	query.exec();

	if (query.first()) {
		QSqlRecord record = query.record();
		s_section = query.value(record.indexOf("section")).toString();
		m_contributionClass = (ContributionClass)query.value(record.indexOf("contribution")).toInt();
		s_nickname = query.value(record.indexOf("nickname")).toString();
		m_gender = (Gender)query.value(record.indexOf("gender")).toInt();
		s_familyName = query.value(record.indexOf("familyname")).toString();
		s_givenName = query.value(record.indexOf("givenname")).toString();
		s_street = query.value(record.indexOf("address")).toString();
		s_postalCode = query.value(record.indexOf("plz")).toString();
		s_city = query.value(record.indexOf("city")).toString();
		s_country = query.value(record.indexOf("country")).toString();
		s_state = query.value(record.indexOf("state")).toString();
		d_birthdate = query.value(record.indexOf("birthday")).toDate();
		d_joining = query.value(record.indexOf("joining")).toDate();
		m_language = (Language)query.value(record.indexOf("language")).toInt();
		m_type = (PersonType)query.value(record.indexOf("type")).toInt();
		m_notify = (NotifyMethod)query.value(record.indexOf("notify_method")).toInt();

		// Load telephone, mobile and email
		query.prepare("SELECT number FROM ldap_persons_telephone WHERE uid=?;");
		query.bindValue(0, uid);
		query.exec();
		while (query.next()) {
			l_telephone << Formatter::telephone(query.value(0).toString());
		}

		query.prepare("SELECT number FROM ldap_persons_mobile WHERE uid=?;");
		query.bindValue(0, uid);
		query.exec();
		while (query.next()) {
			l_mobile << Formatter::telephone(query.value(0).toString());
		}

		query.prepare("SELECT mail FROM ldap_persons_email WHERE uid=?;");
		query.bindValue(0, uid);
		query.exec();
		while (query.next()) {
			l_email << query.value(0).toString();
		}

		_invoice.loadLast(uid);
		is_loaded = TRUE;
	}

	// Even we have no personal data, we load the dates because on importing from LDAP we need them.
	// Otherwise there might be a lack of data.
	loadPaidDueDates();

	return isLoaded();
}

void PPSPerson::loadPaidDueDates() {
	QSqlQuery query(db);
	query.prepare("SELECT paid_due FROM ldap_persons_dates WHERE uid=? ORDER BY paid_due DESC;");
	query.bindValue(0, i_uid);
	query.exec();
	QDate d;
	while (query.next()) {
		d = query.value(0).toDate();
		if (!l_paidDue.contains(d)) {
			l_paidDue << d;
		}
	}
}

bool PPSPerson::loadByPersonsFields(QString uid) {
	QSqlQuery query(db);
	query.prepare("SELECT uid FROM ldap_persons WHERE uid=:val OR nickname=:val OR familyname=:val OR givenname=:val OR (givenname||' '||familyname)=:val OR (familyname||' '||givenname)=:val;");
	query.bindValue(":val", uid);
	if (query.exec() && query.first()) {
		return load(query.value(0).toInt());
	}
	return load(0);
}

bool PPSPerson::loadByTelephone(QString uid) {
	QSqlQuery query(db);
	query.prepare("SELECT uid FROM ldap_persons_mobile WHERE number LIKE :val;");
	query.bindValue(":val", "%" + uid + "%");
	if (query.exec() && query.first()) {
		return load(query.value(0).toInt());
	}

	query.prepare("SELECT uid FROM ldap_persons_telephone WHERE number LIKE :val;");
	query.bindValue(":val", "%" + uid + "%");
	if (query.exec() && query.first()) {
		return load(query.value(0).toInt());
	}
	return load(0);
}

bool PPSPerson::loadByEmail(QString uid) {
	QSqlQuery query(db);

	query.prepare("SELECT uid FROM ldap_persons_email WHERE mail=:val;");
	query.bindValue(":val", uid);
	if (query.exec() && query.first()) {
		return load(query.value(0).toInt());
	}
	return load(0);
}

bool PPSPerson::loadByAnyField(QString uid) {
	return loadByPersonsFields(uid) || loadByEmail(uid) || loadByTelephone(uid);
}

Invoice *PPSPerson::getInvoice() {
	return &_invoice;
}

QList<Invoice *> PPSPerson::getInvoices() {
	return Invoice::getInvoicesForMember(uid());
}

void PPSPerson::setUid(int uid) {
	i_uid = uid;
	emit uidChanged(uid);

	loadPaidDueDates();
	emit paidDueListChanged(l_paidDue);
}

void PPSPerson::setSection(QString section) {
	s_section = section;
	emit sectionChanged(section);
}

void PPSPerson::setContributionClass(ContributionClass contributionClass) {
	m_contributionClass = contributionClass;
	emit contributionClassChanged(contributionClass);
}

void PPSPerson::setNickname(QString nickname) {
	s_nickname = nickname;
	emit nicknameChanged(nickname);
}

void PPSPerson::setGender(Gender gender) {
	m_gender = gender;
	emit genderChanged(gender);
}

void PPSPerson::setFamilyName(QString familyName) {
	s_familyName = familyName;
	emit familyNameChanged(familyName);
}

void PPSPerson::setGivenName(QString givenName) {
	s_givenName = givenName;
	emit givenNameChanged(givenName);
}

void PPSPerson::setStreet(QString street) {
	s_street = street;
	emit streetChanged(street);
}

void PPSPerson::setPostalCode(QString postalCode) {
	s_postalCode = postalCode;
	emit postalCodeChanged(postalCode);
}

void PPSPerson::setCity(QString city) {
	s_city = city;
	emit cityChanged(city);
}

void PPSPerson::setCountry(QString country) {
	s_country = country;
	emit countryChanged(country);
}

void PPSPerson::setState(QString state) {
	s_state = state;
	emit stateChanged(state);
}

void PPSPerson::addTelephone(QString number) {
	number = Formatter::telephone(number);
	l_telephone.append(number);
	emit telephoneAdded(number);
}

void PPSPerson::removeTelephone(QString number) {
	number = Formatter::telephone(number);
	int pos = l_telephone.indexOf(number);
	if (pos >= 0 && pos < l_telephone.size()) {
		l_telephone.removeAt(pos);
		emit telephoneRemoved(number);
	}
}

void PPSPerson::updateTelephone(QString oldNumber, QString newNumber) {
	oldNumber = Formatter::telephone(oldNumber);
	newNumber = Formatter::telephone(newNumber);
	int pos = l_telephone.indexOf(oldNumber);
	if (pos >= 0 && pos < l_telephone.size()) {
		l_telephone.replace(pos, newNumber);
		emit telephoneUpdated(oldNumber, newNumber);
	}
}

void PPSPerson::setTelephone(QList<QString> numbers) {
	for (int i = 0; i < numbers.size(); i++) {
		numbers[i] = Formatter::telephone(numbers[i]);
	}
	l_telephone.clear();
	l_telephone.append(numbers);
	emit telephoneChanged(l_telephone);
}

void PPSPerson::addMobile(QString number) {
	number = Formatter::telephone(number);
	l_mobile.append(number);
	emit mobileAdded(number);
}

void PPSPerson::removeMobile(QString number) {
	number = Formatter::telephone(number);
	int pos = l_mobile.indexOf(number);
	if (pos >= 0 && pos < l_mobile.size()) {
		l_mobile.removeAt(pos);
		emit mobileRemoved(number);
	}
}

void PPSPerson::updateMobile(QString oldNumber, QString newNumber) {
	oldNumber = Formatter::telephone(oldNumber);
	newNumber = Formatter::telephone(newNumber);
	int pos = l_mobile.indexOf(oldNumber);
	if (pos >= 0 && pos < l_mobile.size()) {
		l_mobile.replace(pos, newNumber);
		emit mobileUpdated(oldNumber, newNumber);
	}
}

void PPSPerson::setMobile(QList<QString> numbers) {
	for (int i = 0; i < numbers.size(); i++) {
		numbers[i] = Formatter::telephone(numbers[i]);
	}
	l_mobile.clear();
	l_mobile.append(numbers);
	emit mobileChanged(l_mobile);
}

void PPSPerson::addEmail(QString address) {
	address = Formatter::email(address);
	l_email.append(address);
	emit emailAdded(address);
}

void PPSPerson::removeEmail(QString address) {
	address = Formatter::email(address);
	int pos = l_email.indexOf(address);
	if (pos >= 0 && pos < l_email.size()) {
		l_email.removeAt(pos);
		emit emailRemoved(address);
	}
}

void PPSPerson::updateEmail(QString oldAddress, QString newAddress) {
	oldAddress = Formatter::email(oldAddress);
	newAddress = Formatter::email(newAddress);
	int pos = l_email.indexOf(oldAddress);
	if (pos >= 0 && pos < l_email.size()) {
		l_email.replace(pos, newAddress);
		emit emailUpdated(oldAddress, newAddress);
	}
}

void PPSPerson::setEmail(QList<QString> addresses) {
	for (int i = 0; i < addresses.size(); i++) {
		addresses[i] = Formatter::telephone(addresses[i]);
	}
	l_email.clear();
	l_email.append(addresses);
	emit emailChanged(l_email);
}

void PPSPerson::setBirthdate(QDate birthdate) {
	d_birthdate = birthdate;
	emit birthdateChanged(birthdate);
}

void PPSPerson::setJoining(QDate joining) {
	d_joining = joining;
	emit joiningChanged(joining);
}

void PPSPerson::setLanguage(Language language) {
	m_language = language;
	emit languageChanged(language);
}

void PPSPerson::setPaidDueList(QList<QDate> paidDue) {
	l_paidDue = paidDue;
	if (l_paidDue.isEmpty()) {
		l_paidDue.append(QDate(1970, 1, 1));
	}
	qSort(l_paidDue);
	emit paidDueChanged(l_paidDue.first());
	emit paidDueListChanged(l_paidDue);
}

void PPSPerson::setPaidDue(QDate paidDue) {
	if (!l_paidDue.contains(paidDue)) {
		l_paidDue.append(paidDue);
		qSort(l_paidDue);
		emit paidDueChanged(paidDue);
		emit paidDueListChanged(l_paidDue);
	}
}

void PPSPerson::setType(PersonType type) {
	m_type = type;
	emit typeChanged(type);
}

void PPSPerson::setNotify(NotifyMethod notify) {
	m_notify = notify;
	notifyChanged(notify);
}
