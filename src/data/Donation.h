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


#ifndef Donation_H
#define Donation_H

#include "data/Person.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>

class Donation : public QObject {
Q_OBJECT
Q_PROPERTY(qint32 id READ id)
Q_PROPERTY(qint32 person READ person WRITE setPerson NOTIFY personChanged)
Q_PROPERTY(float amount READ amount WRITE setAmount NOTIFY amountChanged)
Q_PROPERTY(QDate booked READ booked WRITE setBooked NOTIFY bookedChanged)
Q_PROPERTY(QString forSection READ forSection WRITE setForSection NOTIFY forSectionChanged)
Q_PROPERTY(QDate contributed READ contributed WRITE setContributed NOTIFY contributedChanged)
Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
Q_PROPERTY(QString memo READ memo WRITE setMemo NOTIFY memoChanged)
Q_PROPERTY(QString objective READ objective WRITE setObjective NOTIFY objectiveChanged)

// Persons data at the time the donation was given
Q_PROPERTY(QString addressPrefix READ addressPrefix WRITE setAddressPrefix NOTIFY addressPrefixChanged)
Q_PROPERTY(QString addressCompany READ addressCompany WRITE setAddressCompany NOTIFY addressCompanyChanged)
Q_PROPERTY(QString addressName READ addressName WRITE setAddressName NOTIFY addressNameChanged)
Q_PROPERTY(QString addressStreet1 READ addressStreet1 WRITE setAddressStreet1 NOTIFY addressStreet1Changed)
Q_PROPERTY(QString addressStreet2 READ addressStreet2 WRITE setAddressStreet2 NOTIFY addressStreet2Changed)
Q_PROPERTY(QString addressCity READ addressCity WRITE setAddressCity NOTIFY addressCityChanged)
Q_PROPERTY(QString addressCountry READ addressCountry WRITE setAddressCountry NOTIFY addressCountryChanged)
Q_PROPERTY(QString addressEmail READ addressEmail WRITE setAddressEmail NOTIFY addressEmailChanged)
Q_PROPERTY(QString addressPhone READ addressPhone WRITE setAddressPhone NOTIFY addressPhoneChanged)
Q_PROPERTY(QString addressMobile READ addressMobile WRITE setAddressMobile NOTIFY addressMobileChanged)

public:
	Donation(QObject *parent = 0);
	Donation(qint32 id, QObject *parent = 0);
	Donation(const Donation &other);
	virtual ~Donation();
	static void createTables();
	void load(qint32 id);
	void save();
	void deleteItem();
	
	// Setter
	void setPerson(qint32 person);
	void setAmount(float amount);
	void setBooked(QDate date);
	void setForSection(QString section);
	void setContributed(QDate date);
	void setSource(QString source);
	void setMemo(QString memo);
	void setObjective(QString objective);
	
	void setAddressPrefix(QString addressPrefix);
	void setAddressCompany(QString addressCompany);
	void setAddressName(QString addressName);
	void setAddressStreet1(QString addressStreet1);
	void setAddressStreet2(QString addressStreet2);
	void setAddressCity(QString addressCity);
	void setAddressCountry(QString addressCountry);
	void setAddressEmail(QString addressEmail);
	void setAddressPhone(QString addressPhone);
	void setAddressMobile(QString addressMobile);
	
	// Getter
	qint32 id() const { return entityId; };
	qint32 person() const { return i_donator; };
	float amount() const { return f_amount; };
	QDate booked() const { return d_booked; };
	QString forSection() const { return s_forSection; };
	QDate contributed() const { return d_condtributed; };
	QString source() const { return s_source; };
	QString memo() const { return s_memo; };
	QString objective() const { return s_objective; };
	QString addressPrefix() const { return s_addressPrefix; };
	QString addressCompany() const { return s_addressCompany; };
	QString addressName() const { return s_addressName; };
	QString addressStreet1() const { return s_addressStreet1; };
	QString addressStreet2() const { return s_addressStreet2; };
	QString addressCity() const { return s_addressCity; };
	QString addressCountry() const { return s_addressCountry; };
	QString addressEmail() const { return s_addressEmail; };
	QString addressPhone() const { return s_addressPhone; };
	QString addressMobile() const { return s_addressMobile; };
	
signals:
	void personChanged(qint32);
	void amountChanged(float);
	void bookedChanged(QDate);
	void forSectionChanged(QString);
	void contributedChanged(QDate);
	void sourceChanged(QString);
	void memoChanged(QString);
	void objectiveChanged(QString);
	void addressPrefixChanged(QString);
	void addressCompanyChanged(QString);
	void addressNameChanged(QString);
	void addressStreet1Changed(QString);
	void addressStreet2Changed(QString);
	void addressCityChanged(QString);
	void addressCountryChanged(QString);
	void addressEmailChanged(QString);
	void addressPhoneChanged(QString);
	void addressMobileChanged(QString);
	
private:
	qint32 entityId;
	qint32 i_donator;
	float f_amount;
	QDate d_booked;
	QString s_forSection;
	QDate d_condtributed;
	QString s_source;
	QString s_memo;
	QString s_objective;
	QString s_addressPrefix;
	QString s_addressCompany;
	QString s_addressName;
	QString s_addressStreet1;
	QString s_addressStreet2;
	QString s_addressCity;
	QString s_addressCountry;
	QString s_addressEmail;
	QString s_addressPhone;
	QString s_addressMobile;
	
	QSqlDatabase db;
	bool _loaded;
	void clear();
};

#endif // Donation_H
