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

#ifndef Data_Person_H
#define Data_Person_H

#include "Invoice.h"

#include <QObject>
#include <QString>
#include <QDate>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

class PPSPerson : public QObject {
Q_OBJECT
Q_PROPERTY(int uid READ uid WRITE setUid NOTIFY uidChanged)
Q_PROPERTY(QString section READ section WRITE setSection NOTIFY sectionChanged)
Q_PROPERTY(ContributionClass contributionClass READ contributionClass WRITE setContributionClass NOTIFY contributionClassChanged)
Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
Q_PROPERTY(Gender gender READ gender WRITE setGender NOTIFY genderChanged)
Q_PROPERTY(QString familyName READ familyName WRITE setFamilyName NOTIFY familyNameChanged)
Q_PROPERTY(QString givenName READ givenName WRITE setGivenName NOTIFY givenNameChanged)
Q_PROPERTY(QString street READ street WRITE setStreet NOTIFY streetChanged)
Q_PROPERTY(QString postalCode READ postalCode WRITE setPostalCode NOTIFY postalCodeChanged)
Q_PROPERTY(QString city READ city WRITE setCity NOTIFY cityChanged)
Q_PROPERTY(QString country READ country WRITE setCountry NOTIFY countryChanged)
Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
Q_PROPERTY(QList<QString> telephone READ telephone WRITE setTelephone NOTIFY telephoneChanged)
Q_PROPERTY(QList<QString> mobile READ mobile WRITE setMobile NOTIFY mobileChanged)
Q_PROPERTY(QList<QString> email READ email WRITE setEmail NOTIFY emailChanged)
Q_PROPERTY(QDate birthdate READ birthdate WRITE setBirthdate NOTIFY birthdateChanged)
Q_PROPERTY(QDate joining READ joining WRITE setJoining NOTIFY joiningChanged)
Q_PROPERTY(Language language READ language WRITE setLanguage NOTIFY languageChanged)
Q_PROPERTY(QList<QDate> paidDueList READ paidDueList WRITE setPaidDueList NOTIFY paidDueListChanged)
Q_PROPERTY(QDate paidDue READ paidDue WRITE setPaidDue NOTIFY paidDueChanged)
Q_PROPERTY(PersonType type READ type WRITE setType NOTIFY typeChanged)
Q_PROPERTY(NotifyMethod notify READ notify WRITE setNotify NOTIFY notifyChanged)

Q_ENUMS(ContributionClass)
Q_ENUMS(Gender)
Q_ENUMS(Language)
Q_ENUMS(PersonType)
Q_ENUMS(NotifyMethod)

public:
	PPSPerson(QObject *parent = 0);
	PPSPerson(const PPSPerson &other);
	virtual ~PPSPerson();
	void save();
	void clear();
	bool load(int uid);
	bool loadByPersonsFields(QString uid);
	bool loadByTelephone(QString uid);
	bool loadByEmail(QString uid);
	bool loadByAnyField(QString uid);
	Invoice *getInvoice();
	QList<Invoice *> getInvoices();
	static void createTables();
	static void emptyTables();
	static void resetMembertypes();

	enum ContributionClass { ContributeFull=0 , ContributeStudent=1 };
	enum Gender { GenderMale=0, GenderFemale=1, GenderBoth=2, GenderNone=3, GenderUnknown=-1 };
	enum Language { DE=0, FR=1, IT=2, EN=3 };
	enum PersonType { LandLubber=0, Sympathizer=1, Pirate=2, Veteran=3, PlankWalker=4, FleetPirate=5, UnknownType=99 };
	enum NotifyMethod { SnailMail=0, EMail=1 };

	// Setter
	void setUid(int uid);
	void setSection(QString section);
	void setContributionClass(ContributionClass contributionClass);
	void setNickname(QString nickname);
	void setGender(Gender gender);
	void setFamilyName(QString familyName);
	void setGivenName(QString givenName);
	void setStreet(QString street);
	void setPostalCode(QString postalCode);
	void setCity(QString city);
	void setCountry(QString country);
	void setState(QString state);
	void addTelephone(QString number);
	void removeTelephone(QString number);
	void updateTelephone(QString oldNumber, QString newNumber);
	void setTelephone(QList<QString> numbers);
	void addMobile(QString number);
	void removeMobile(QString number);
	void updateMobile(QString oldNumber, QString newNumber);
	void setMobile(QList<QString> numbers);
	void addEmail(QString address);
	void removeEmail(QString address);
	void updateEmail(QString oldAddress, QString newAddress);
	void setEmail(QList<QString> addresses);
	void setBirthdate(QDate birthdate);
	void setJoining(QDate joining);
	void setLanguage(Language language);
	void setPaidDueList(QList<QDate> paidDue);
	void setPaidDue(QDate paidDue);
	void setType(PersonType type);
	void setNotify(NotifyMethod notify);

	// Getter
	bool isLoaded() { return i_uid > 0; };
	int uid() const { return i_uid; };
	QString section() const { return s_section; };
	ContributionClass contributionClass() const { return m_contributionClass; };
	QString nickname() const { return s_nickname; };
	Gender gender() const { return m_gender; };
	QString familyName() const { return s_familyName; };
	QString givenName() const { return s_givenName; };
	QString street() const { return s_street; };
	QString postalCode() const { return s_postalCode; };
	QString city() const { return s_city; };
	QString country() const { return s_country; };
	QString state() const { return s_state; };
	QList<QString> telephone() const { return l_telephone; };
	QList<QString> mobile() const { return l_mobile; };
	QList<QString> email() const { return l_email; };
	QDate birthdate() const { return d_birthdate; };
	QDate joining() const { return d_joining; };
	Language language() const { return m_language; };
	QList<QDate> paidDueList() const { return l_paidDue; };
	QDate paidDue() const { return l_paidDue.last(); };
	PersonType type() const { return m_type; };
	NotifyMethod notify() const { return m_notify; };

signals:
	void uidChanged(int);
	void sectionChanged(QString);
	void contributionClassChanged(ContributionClass);
	void nicknameChanged(QString);
	void genderChanged(Gender);
	void familyNameChanged(QString);
	void givenNameChanged(QString);
	void streetChanged(QString);
	void postalCodeChanged(QString);
	void cityChanged(QString);
	void countryChanged(QString);
	void stateChanged(QString);
	void telephoneAdded(QString);
	void telephoneRemoved(QString);
	void telephoneUpdated(QString, QString);
	void telephoneChanged(QList<QString>);
	void mobileAdded(QString);
	void mobileRemoved(QString);
	void mobileUpdated(QString, QString);
	void mobileChanged(QList<QString>);
	void emailAdded(QString);
	void emailRemoved(QString);
	void emailUpdated(QString, QString);
	void emailChanged(QList<QString>);
	void birthdateChanged(QDate);
	void joiningChanged(QDate);
	void languageChanged(Language);
	void paidDueListChanged(QList<QDate>);
	void paidDueChanged(QDate);
	void typeChanged(PersonType);
	void notifyChanged(NotifyMethod);

private:
	QSqlDatabase db;
	int i_uid;
	QString s_section;
	ContributionClass m_contributionClass;
	QString s_nickname;
	Gender m_gender;
	QString s_familyName;
	QString s_givenName;
	QString s_street;
	QString s_postalCode;
	QString s_city;
	QString s_country;
	QString s_state;
	QList<QString> l_telephone;
	QList<QString> l_mobile;
	QList<QString> l_email;
	QDate d_birthdate;
	QDate d_joining;
	Language m_language;
	Invoice _invoice;
	QList<QDate> l_paidDue;
	PersonType m_type;
	NotifyMethod m_notify;
	bool is_loaded;

protected:
	void loadPaidDueDates();

};

#endif // Data_Person_H
