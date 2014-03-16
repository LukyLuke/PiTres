/*
	An invoice
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

#ifndef Data_Invoice_H
#define Data_Invoice_H

#include "helper/Formatter.h"
#include "helper/XmlPdf.h"
#include "data/Section.h"

#include <cstdlib>
#include <ctime>
#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDate>
#include <QList>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlRecord>

class PPSPerson;
class Invoice : public QObject {
Q_OBJECT
Q_PROPERTY(int memberUid READ memberUid WRITE setMemberUid NOTIFY memberUidChanged)
Q_PROPERTY(QString reference READ reference WRITE setReference NOTIFY referenceChanged)
Q_PROPERTY(QDate issueDate READ issueDate WRITE setIssueDate NOTIFY issueDateChanged)
Q_PROPERTY(QDate payableDue READ payableDue WRITE setPayableDue NOTIFY payableDueChanged)
Q_PROPERTY(QDate paidDate READ paidDate WRITE setPaidDate NOTIFY paidDateChanged)
Q_PROPERTY(float amount READ amount WRITE setAmount NOTIFY amountChanged)
Q_PROPERTY(float amountPaid READ amountPaid WRITE setAmountPaid NOTIFY amountPaidChanged)
Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
Q_PROPERTY(int reminded READ reminded WRITE setReminded NOTIFY remindedChanged)
Q_PROPERTY(QDate lastReminded READ lastReminded WRITE setLastReminded NOTIFY lastRemindedChanged)

// This is the owner
Q_PROPERTY(QString addressPrefix READ addressPrefix WRITE setAddressPrefix NOTIFY addressPrefixChanged)
Q_PROPERTY(QString addressCompany READ addressCompany WRITE setAddressCompany NOTIFY addressCompanyChanged)
Q_PROPERTY(QString addressName READ addressName WRITE setAddressName NOTIFY addressNameChanged)
Q_PROPERTY(QString addressStreet1 READ addressStreet1 WRITE setAddressStreet1 NOTIFY addressStreet1Changed)
Q_PROPERTY(QString addressStreet2 READ addressStreet2 WRITE setAddressStreet2 NOTIFY addressStreet2Changed)
Q_PROPERTY(QString addressCity READ addressCity WRITE setAddressCity NOTIFY addressCityChanged)
Q_PROPERTY(QString addressCountry READ addressCountry WRITE setAddressCountry NOTIFY addressCountryChanged)
Q_PROPERTY(QString addressEmail READ addressEmail WRITE setAddressEmail NOTIFY addressEmailChanged)
Q_PROPERTY(QString forSection READ forSection WRITE setForSection NOTIFY forSectionChanged)
Q_PROPERTY(Language language READ language WRITE setLanguage NOTIFY languageChanged)

Q_ENUMS(State)
Q_ENUMS(Language)

public:
	Invoice(QObject *parent = 0);
	virtual ~Invoice();
	void save();
	void clear();
	void setIsLoaded(bool loaded);
	bool isLoaded();
	void loadLast(int member);
	XmlPdf *createPdf(QString tpl = 0);
	QString getEsr();
	void create(PPSPerson *person);
	void load(QString reference);
	void loadByReference(QString reference);
	bool pay(QDate *date = 0);
	bool pay(float amount, QDate *date = 0);
	static QList<Invoice *> getInvoicesForMember(int member);
	static void createTables();
	static void setContributed(QString reference);
	static void synchronizeWithMembers();

	// InvoiceState in old DB: o_pen, c_anceled, p_aid, u_nknown
	enum State { StateOpen=0, StateCanceled=1, StatePaid=2, StateUnknown=3, StateContributed=4 };
	enum Language { DE=0, FR=1, IT=2, EN=3 };

	// Setter
	void setMemberUid(int memberUid);
	void setReference(QString reference);
	void setIssueDate(QDate issueDate);
	void setPayableDue(QDate payableDue);
	void setPaidDate(QDate paidDate);
	void setAmount(float amount);
	void setAmountPaid(float amountPaid);
	void setState(State state);
	void setReminded(int reminded);
	void setLastReminded(QDate lastReminded);
	void setAddressPrefix(QString addressPrefix);
	void setAddressPrefix(int gender);
	void setAddressCompany(QString addressCompany);
	void setAddressName(QString addressName);
	void setAddressStreet1(QString addressStreet1);
	void setAddressStreet2(QString addressStreet2);
	void setAddressCity(QString addressCity);
	void setAddressCountry(QString addressCountry);
	void setAddressEmail(QString addressEmail);
	void setForSection(QString forSection);
	void setLanguage(Language language);
#ifdef FIO
	void setRecommendations(QHash<QString, float> recommendations);
#endif

	// Getter
	int memberUid() const { return i_memberUid; };
	QString reference();// const { return s_reference; }; // Reformatted
	QString referencePlain() const { return s_reference; };
	QDate issueDate() const { return d_issueDate; };
	QDate payableDue() const { return d_payableDue; };
	QDate paidDate() const { return d_paidDate; };
	float amount() const { return f_amount; };
	float amountPaid() const { return f_amountPaid; };
	State state() const { return m_state; };
	bool isPaid() const { return m_state == StatePaid; };
	int reminded() const { return i_reminded; };
	QDate lastReminded() const { return d_last_reminded; };
	QString addressPrefix() const { return s_addressPrefix; };
	QString addressCompany() const { return s_addressCompany; };
	QString addressName() const { return s_addressName; };
	QString addressStreet1() const { return s_addressStreet1; };
	QString addressStreet2() const { return s_addressStreet2; };
	QString addressCity() const { return s_addressCity; };
	QString addressCountry() const { return s_addressCountry; };
	QString addressEmail() const { return s_addressEmail; };
	QString forSection() const { return s_forSection; };
	Language language() const { return m_language; };
#ifdef FIO
	QHash<QString, float> recommendations() const { return l_recom; };
#endif

signals:
	void memberUidChanged(int);
	void referenceChanged(QString);
	void issueDateChanged(QDate);
	void payableDueChanged(QDate);
	void paidDateChanged(QDate);
	void amountChanged(float);
	void amountPaidChanged(float);
	void stateChanged(State);
	void remindedChanged(int);
	void lastRemindedChanged(QDate);
	void addressPrefixChanged(QString);
	void addressCompanyChanged(QString);
	void addressNameChanged(QString);
	void addressStreet1Changed(QString);
	void addressStreet2Changed(QString);
	void addressCityChanged(QString);
	void addressCountryChanged(QString);
	void addressEmailChanged(QString);
	void forSectionChanged(QString);
	void languageChanged(Language);
#ifdef FIO
	void recommendationsChanged(QHash<QString, float>);
#endif

private:
	QSqlDatabase db;
	bool _loaded;
	int i_memberUid;
	QString s_reference;
	QDate d_issueDate;
	QDate d_payableDue;
	QDate d_paidDate;
	float f_amount;
	float f_amountPaid;
	State m_state;
	int i_reminded;
	QDate d_last_reminded;
	QString s_addressPrefix;
	QString s_addressCompany;
	QString s_addressName;
	QString s_addressStreet1;
	QString s_addressStreet2;
	QString s_addressCity;
	QString s_addressCountry;
	QString s_addressEmail;
	QString s_forSection;
	Language m_language;
#ifdef FIO
	QHash<QString, float> l_recom;
	float setRecommendations(QString section);
#endif
	
	int esrChecksum(QString num);
	QString createReference(int memberUid);
	QString getLanguageString(Language lang);
	void setProperties(QSqlQuery *record);
};

#endif // Data_Invoice_H
