
#include "Person.h"
#include "helper/Formatter.h"

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

PPSPerson::~PPSPerson() {}

void PPSPerson::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons (uid INTEGER, contribution INTEGER, nickname TEXT, gender TEXT, familyname TEXT, "
	              "givenname TEXT, address TEXT, plz TEXT, city TEXT, country TEXT, state TEXT, birthday DATE, language TEXT, joining DATE, section TEXT, "
	              "ldap_paid_due DATE, type INTEGER);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_email (uid INTEGER, type TEXT, mail TEXT);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_telephone (uid INTEGER, type TEXT, number TEXT);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_mobile (uid INTEGER, type TEXT, number TEXT);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	// Persistent matching table between PaidUntilDates and MemberUIDs
	query.prepare("CREATE TABLE IF NOT EXISTS ldap_persons_dates (uid INTEGER, paid_due DATE);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
}

void PPSPerson::emptyTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("DELETE FROM ldap_persons;");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("DELETE FROM ldap_persons_email;");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("DELETE FROM ldap_persons_telephone;");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("DELETE FROM ldap_persons_mobile;");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	query.prepare("VACUUM;");
	query.exec();
}

void PPSPerson::save() {
	int i;
	QSqlQuery query(db);
	
	// Persons are Readonly, expect when they are loaded from LDAP
	if (!is_loaded) {
		query.prepare("INSERT INTO ldap_persons (uid, contribution, nickname, gender, familyname, givenname, address, plz, city, country, state, birthday, language, joining, section, ldap_paid_due, type) "
		              "VALUES (:uid, :contcls, :nickname, :gender, :familyname, :givenname, :address, :plz, :city, :country, :state, :birthday, :language, :joining, :section, :ldappaiddue, :type);");
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
		query.bindValue(":ldappaiddue", d_ldapPaidDue);
		query.bindValue(":type", (int)m_type);
		query.exec();
		
		if (query.lastError().type() != QSqlError::NoError) {
			qDebug() << query.lastQuery();
			qDebug() << query.lastError();
		}
		
		// Insert all telephones, mobiles and email addresses
		if (!l_telephone.empty()) {
			query.prepare("INSERT INTO ldap_persons_telephone (uid, type, number) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_telephone.size(); i++) {
				query.bindValue(":value", l_telephone.at(i));
				query.exec();
				if (query.lastError().type() != QSqlError::NoError) {
					qDebug() << query.lastQuery();
					qDebug() << query.lastError();
				}
			}
		}
		
		if (!l_mobile.empty()) {
			query.prepare("INSERT INTO ldap_persons_mobile (uid, type, number) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_mobile.size(); i++) {
				query.bindValue(":value", l_mobile.at(i));
				query.exec();
				if (query.lastError().type() != QSqlError::NoError) {
					qDebug() << query.lastQuery();
					qDebug() << query.lastError();
				}
			}
		}
		
		if (!l_email.empty()) {
			query.prepare("INSERT INTO ldap_persons_email (uid, type, mail) VALUES (:uid, :type, :value);");
			query.bindValue(":uid", i_uid);
			query.bindValue(":type", "default");
			for (i = 0; i < l_email.size(); i++) {
				query.bindValue(":value", l_email.at(i));
				query.exec();
				if (query.lastError().type() != QSqlError::NoError) {
					qDebug() << query.lastQuery();
					qDebug() << query.lastError();
				}
			}
		}
		
		// Save the PaidDue matching Table
		query.prepare("INSERT INTO ldap_persons_dates (uid,paid_due) VALUES (:uid,:due);");
		query.bindValue(":uid", i_uid);
		query.bindValue(":due", d_paidDue);
		query.exec();
		
	} else {
		// Update the PaidDue matching Table
		query.prepare("UPDATE ldap_persons_dates SET paid_due=:due WHERE uid=:uid;");
		query.bindValue(":uid", i_uid);
		query.bindValue(":due", d_paidDue);
		query.exec();
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
	d_birthdate = QDate(1970, 01, 01);
	d_joining = QDate::currentDate();
	m_language = EN;
	d_paidDue = QDate(1970, 01, 01);
	m_type = Pirate;
	_invoice.clear();
}

bool PPSPerson::load(int uid) {
	clear();
	QSqlQuery query(db);
	query.prepare("SELECT * FROM ldap_persons WHERE uid=?;");
	query.bindValue(0, uid);
	query.exec();

	if (query.first()) {
		QSqlRecord record = query.record();
		i_uid = query.value(record.indexOf("uid")).toInt();
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
		
		query.prepare("SELECT paid_due FROM ldap_persons_dates WHERE uid=?;");
		query.bindValue(0, uid);
		query.exec();
		if (query.next()) {
			d_paidDue = query.value(0).toDate();
		}
		
		_invoice.loadLast(uid);
	}
	is_loaded = TRUE;
	return isLoaded();
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
	l_telephone.append(number);
	emit telephoneAdded(number);
}

void PPSPerson::removeTelephone(QString number) {
	int pos = l_telephone.indexOf(number);
	if (pos >= 0 && pos < l_telephone.size()) {
		l_telephone.removeAt(pos);
		emit telephoneRemoved(number);
	}
}

void PPSPerson::updateTelephone(QString oldNumber, QString newNumber) {
	int pos = l_telephone.indexOf(oldNumber);
	if (pos >= 0 && pos < l_telephone.size()) {
		l_telephone.replace(pos, newNumber);
		emit telephoneUpdated(oldNumber, newNumber);
	}
}

void PPSPerson::setTelephone(QList<QString> numbers) {
	l_telephone.clear();
	l_telephone.append(numbers);
	emit telephoneChanged(l_telephone);
}

void PPSPerson::addMobile(QString number) {
	l_mobile.append(number);
	emit mobileAdded(number);
}

void PPSPerson::removeMobile(QString number) {
	int pos = l_mobile.indexOf(number);
	if (pos >= 0 && pos < l_mobile.size()) {
		l_mobile.removeAt(pos);
		emit mobileRemoved(number);
	}
}

void PPSPerson::updateMobile(QString oldNumber, QString newNumber) {
	int pos = l_mobile.indexOf(oldNumber);
	if (pos >= 0 && pos < l_mobile.size()) {
		l_mobile.replace(pos, newNumber);
		emit mobileUpdated(oldNumber, newNumber);
	}
}

void PPSPerson::setMobile(QList<QString> numbers) {
	l_mobile.clear();
	l_mobile.append(numbers);
	emit mobileChanged(l_mobile);
}

void PPSPerson::addEmail(QString address) {
	l_email.append(address);
	emit emailAdded(address);
}

void PPSPerson::removeEmail(QString address) {
	int pos = l_email.indexOf(address);
	if (pos >= 0 && pos < l_email.size()) {
		l_email.removeAt(pos);
		emit emailRemoved(address);
	}
}

void PPSPerson::updateEmail(QString oldAddress, QString newAddress) {
	int pos = l_email.indexOf(oldAddress);
	if (pos >= 0 && pos < l_email.size()) {
		l_email.replace(pos, newAddress);
		emit emailUpdated(oldAddress, newAddress);
	}
}

void PPSPerson::setEmail(QList<QString> addresses) {
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

void PPSPerson::setPaidDue(QDate paidDue) {
	d_paidDue = paidDue;
	emit paidDueChanged(paidDue);
}

void PPSPerson::setLdapPaidDue(QDate paidDue) {
	d_ldapPaidDue = paidDue;
	emit ldapPaidDueChanged(paidDue);
}

void PPSPerson::setType(PersonType type) {
	m_type = type;
	emit typeChanged(type);
}
