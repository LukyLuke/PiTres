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

Q_ENUMS(ContributionClass)
Q_ENUMS(Gender)
Q_ENUMS(Language)

public:
	PPSPerson(QObject *parent = 0);
	virtual ~PPSPerson();
	void save(QSqlDatabase db);
	void clear();
	void load(QSqlDatabase db, int uid);
	Invoice *getInvoice();
	static void createTables(QSqlDatabase db);
	static void emptyTables(QSqlDatabase db);
	
	enum ContributionClass { ContributeFull=0, ContributeStudent=1 };
	enum Gender { GenderMale=0, GenderFemale=1, GenderBoth=2, GenderNone=3, GenderUnknown=-1 };
	enum Language { DE=0, FR=1, IT=2, EN=3 };
	
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
	
	// Getter
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

private:
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
};

#endif // Data_Person_H