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

#include "Invoice.h"
#include "Person.h"
#include "Section.h"

#include <cstdlib>
#include <ctime>
#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QVariant>
#include <QDate>
#include <QList>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

Invoice::Invoice(QObject *parent) : QObject(parent) {
	srand(time(NULL));
	clear();
}

Invoice::~Invoice() {}

void Invoice::clear() {
	setIsLoaded(TRUE);
	i_memberUid = 0;
	s_reference = "";
	d_issueDate = QDate::currentDate();
	d_payableDue = QDate::currentDate().addDays(30);
	d_paidDate = QDate(1970, 1, 1);
	f_amount = 0.0;
	f_amountPaid = 0.0;
	m_state = StateOpen;
	i_reminded = 0;
	d_last_reminded = QDate(1970, 1, 1);
	s_addressPrefix = "";
	s_addressCompany = "";
	s_addressName = "";
	s_addressStreet1 = "";
	s_addressStreet2 = "";
	s_addressCity = "";
	s_addressCountry = "";
	s_addressEmail = "";
	s_forSection = "";
#ifdef FIO
	l_recom.clear();
#endif
}

void Invoice::setIsLoaded(bool loaded) {
	_loaded = loaded;
}

void Invoice::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS pps_invoice (member_uid INTEGER, reference TEXT, issue_date DATE, payable_date DATE, paid_date DATE, "
	              "amount FLOAT, amount_paid FLOAT, state INTEGER, address_prefix TEXT, address_company TEXT, address_name TEXT, address_street1 TEXT, "
	              "address_street2 TEXT, address_city TEXT, address_email TEXT, for_section TEXT, reminded INTEGER, last_reminder DATE, recommendations TEXT);");
	query.exec();
}

void Invoice::setContributed(QString reference) {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("UPDATE pps_invoice SET state=:state WHERE reference=:reference;");
	query.bindValue(":state", StateContributed);
	query.bindValue(":reference", reference);
	query.exec();
}

void Invoice::save() {
	if (i_memberUid <= 0) return;
	QSqlQuery query(db);
	if (_loaded) {
		query.prepare("UPDATE pps_invoice SET member_uid=:member,reference=:reference,issue_date=:issued,payable_date=:payable,paid_date=:paid,"
		              "amount=:amount,amount_paid=:amount_paid,state=:state,address_prefix=:prefix,address_company=:company,address_name=:name,"
		              "address_street1=:street1,address_street2=:street2,address_city=:city,address_email=:email,for_section=:section,"
		              "reminded=:reminded,last_reminder=:last_reminder,recommendations=:recommendations WHERE reference=:reference_where;");
		query.bindValue(":reference_where", s_reference);
	} else {
		query.prepare("INSERT INTO pps_invoice (member_uid,reference,issue_date,payable_date,paid_date,amount,amount_paid,state,"
		              "address_prefix,address_company,address_name,address_street1,address_street2,address_city,address_email,for_section,"
		              "reminded,last_reminder,recommendations) VALUES (:member,:reference,:issued,:payable,:paid,:amount,:amount_paid,:state,:prefix,:company,:name,"
		              ":street1,:street2,:city,:email,:section,:reminded,:last_reminder,:recommendations);");
	}

	// This is for all old References
	/*QString ref(s_reference);
	if (ref.length() == 16) {
		ref.insert(4, " ");
		ref.insert(9, " ");
		ref.insert(14, " ");
	}*/

	query.bindValue(":member", i_memberUid);
	query.bindValue(":reference", s_reference);
	query.bindValue(":issued", d_issueDate);
	query.bindValue(":payable", d_payableDue);
	query.bindValue(":paid", d_paidDate);
	query.bindValue(":amount", f_amount);
	query.bindValue(":amount_paid", f_amountPaid);
	query.bindValue(":state", (int)m_state);
	query.bindValue(":prefix", s_addressPrefix);
	query.bindValue(":company", s_addressCompany);
	query.bindValue(":name", s_addressName);
	query.bindValue(":street1", s_addressStreet1);
	query.bindValue(":street2", s_addressStreet2);
	query.bindValue(":city", s_addressCity);
	query.bindValue(":email", s_addressEmail);
	query.bindValue(":section", s_forSection);
	query.bindValue(":reminded", i_reminded);
	query.bindValue(":last_reminder", d_last_reminded);
#ifndef FIO
	query.bindValue(":recommendations", "");
#else
	QString rec;
	QHash<QString, float>::const_iterator it = l_recom.constBegin();
	while (it != l_recom.constEnd()) {
		if (!rec.isEmpty()) {
			rec += ";";
		}
		rec += it.key() + ":" + QString::number(it.value());
		it++;
	}
	query.bindValue(":recommendations", rec);
#endif
	query.exec();

	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
		setIsLoaded(false);
	} else {
		setIsLoaded(true);
	}
}

void Invoice::synchronizeWithMembers() {
	QSqlDatabase db;
	QSqlQuery query(db);

	// Close all no longer existent invoices
	query.prepare("UPDATE pps_invoice SET state=:state_new WHERE member_uid IN"
		" (SELECT pps_invoice.member_uid FROM pps_invoice LEFT JOIN ldap_persons ON"
		" (pps_invoice.member_uid = ldap_persons.uid) WHERE pps_invoice.state=:state_old"
		" AND ldap_persons.uid IS NULL);");
	query.bindValue(":state_new", Invoice::StateCanceled);
	query.bindValue(":state_old", Invoice::StateOpen);
	query.exec();
}

void Invoice::loadLast(int member) {
	clear();
	QSqlQuery query(db);
	query.prepare("SELECT * FROM pps_invoice WHERE member_uid=? ORDER BY issue_date DESC;");
	query.bindValue(0, member);
	query.exec();

	_loaded = query.first();
	if (_loaded) {
		QSqlRecord record = query.record();
		i_memberUid = query.value(record.indexOf("member_uid")).toInt();
		setReference(query.value(record.indexOf("reference")).toString()); // we reformat the Reference
		d_issueDate = query.value(record.indexOf("issue_date")).toDate();
		d_payableDue = query.value(record.indexOf("payable_date")).toDate();
		d_paidDate = query.value(record.indexOf("paid_date")).toDate();
		f_amount = query.value(record.indexOf("amount")).toFloat();
		f_amountPaid = query.value(record.indexOf("amount_paid")).toFloat();
		m_state = (State) query.value(record.indexOf("state")).toInt();
		i_reminded = query.value(record.indexOf("reminded")).toInt();
		d_last_reminded = query.value(record.indexOf("last_reminder")).toDate();
		s_addressPrefix = query.value(record.indexOf("address_prefix")).toString();
		s_addressCompany = query.value(record.indexOf("address_company")).toString();
		s_addressName = query.value(record.indexOf("address_name")).toString();
		s_addressStreet1 = query.value(record.indexOf("address_street1")).toString();
		s_addressStreet2 = query.value(record.indexOf("address_street2")).toString();
		s_addressCity = query.value(record.indexOf("address_city")).toString();
		s_addressEmail = query.value(record.indexOf("address_email")).toString();
		s_forSection = query.value(record.indexOf("for_section")).toString();
#ifdef FIO
		QStringList sl,l = query.value(record.indexOf("recommendations")).toString().split(';', QString::SkipEmptyParts);
		for (int i = 0; i < l.length(); i++) {
			sl = l.at(i).split(':');
			if (sl.length() == 2) {
				l_recom.insert(sl[0], sl[1].toFloat());
			} else {
				qDebug() << "Invoice: Error while parsing recommendations, wrong format: '" << l.at(i) << "' should be 'Section name:amount'";
			}
		}
#endif
	}
}

void Invoice::load(QString reference) {
	loadByReference(reference.append("0")); // Add a Pseudo-Checksum
}

void Invoice::loadByReference(QString reference) {
	clear();
	QString ref = QString(reference);
	if (ref.size() > 0) {
		ref.remove(QChar(' '), Qt::CaseInsensitive);
		ref = ref.left(ref.size()-1); // Last Char is the Checksum which isn't in the Database
	}

	// This is for all old References
	if (ref.left(10) == "0000000000") {
		ref = ref.right(16);
		ref.insert(4, " ");
		ref.insert(9, " ");
		ref.insert(14, " ");
	}
	QSqlQuery query(db);
	query.prepare("SELECT * FROM pps_invoice WHERE reference=?;");
	query.bindValue(0, ref);
	query.exec();

	_loaded = query.first();
	if (_loaded) {
		QSqlRecord record = query.record();
		i_memberUid = query.value(record.indexOf("member_uid")).toInt();
		setReference(query.value(record.indexOf("reference")).toString()); // we reformat the Reference
		d_issueDate = query.value(record.indexOf("issue_date")).toDate();
		d_payableDue = query.value(record.indexOf("payable_date")).toDate();
		d_paidDate = query.value(record.indexOf("paid_date")).toDate();
		f_amount = query.value(record.indexOf("amount")).toFloat();
		f_amountPaid = query.value(record.indexOf("amount_paid")).toFloat();
		m_state = (State) query.value(record.indexOf("state")).toInt();
		i_reminded = query.value(record.indexOf("reminded")).toInt();
		d_last_reminded = query.value(record.indexOf("last_reminder")).toDate();
		s_addressPrefix = query.value(record.indexOf("address_prefix")).toString();
		s_addressCompany = query.value(record.indexOf("address_company")).toString();
		s_addressName = query.value(record.indexOf("address_name")).toString();
		s_addressStreet1 = query.value(record.indexOf("address_street1")).toString();
		s_addressStreet2 = query.value(record.indexOf("address_street2")).toString();
		s_addressCity = query.value(record.indexOf("address_city")).toString();
		s_addressEmail = query.value(record.indexOf("address_email")).toString();
		s_forSection = query.value(record.indexOf("for_section")).toString();
#ifdef FIO
		QStringList sl,l = query.value(record.indexOf("recommendations")).toString().split(';', QString::SkipEmptyParts);
		for (int i = 0; i < l.length(); i++) {
			sl = l.at(i).split(':');
			if (sl.length() == 2) {
				l_recom.insert(sl[0], sl[1].toFloat());
			} else {
				qDebug() << "Invoice: Error while parsing recommendations, wrong format: '" << l.at(i) << "' should be 'Section name:amount'";
			}
		}
#endif
	}
}

QList<Invoice *> Invoice::getInvoicesForMember(int member) {
	QList<Invoice *> back;
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("SELECT * FROM pps_invoice WHERE member_uid=? ORDER BY issue_date DESC;");
	query.bindValue(0, member);
	query.exec();
	QSqlRecord record = query.record();

	while(query.next()) {
		Invoice *invoice = new Invoice;
		invoice->setMemberUid(query.value(record.indexOf("member_uid")).toInt());
		invoice->setReference(query.value(record.indexOf("reference")).toString());
		invoice->setIssueDate(query.value(record.indexOf("issue_date")).toDate());
		invoice->setPayableDue(query.value(record.indexOf("payable_date")).toDate());
		invoice->setPaidDate(query.value(record.indexOf("paid_date")).toDate());
		invoice->setAmount(query.value(record.indexOf("amount")).toFloat());
		invoice->setAmountPaid(query.value(record.indexOf("amount_paid")).toFloat());
		invoice->setState((State) query.value(record.indexOf("state")).toInt());
		invoice->setReminded(query.value(record.indexOf("reminded")).toInt());
		invoice->setLastReminded(query.value(record.indexOf("last_reminder")).toDate());
		invoice->setAddressPrefix(query.value(record.indexOf("address_prefix")).toString());
		invoice->setAddressCompany(query.value(record.indexOf("address_company")).toString());
		invoice->setAddressName(query.value(record.indexOf("address_name")).toString());
		invoice->setAddressStreet1(query.value(record.indexOf("address_street1")).toString());
		invoice->setAddressStreet2(query.value(record.indexOf("address_street2")).toString());
		invoice->setAddressCity(query.value(record.indexOf("address_city")).toString());
		invoice->setAddressEmail(query.value(record.indexOf("address_email")).toString());
		invoice->setForSection(query.value(record.indexOf("for_section")).toString());
#ifdef FIO
		QStringList sl,l = query.value(record.indexOf("recommendations")).toString().split(';', QString::SkipEmptyParts);
		QHash<QString, float> hash;
		for (int i = 0; i < l.length(); i++) {
			sl = l.at(i).split(':');
			if (sl.length() == 2) {
				hash.insert(sl[0], sl[1].toFloat());
			} else {
				qDebug() << "Invoice: Error while parsing recommendations, wrong format: '" << l.at(i) << "' should be 'Section name:amount'";
			}
		}
		invoice->setRecommendations(hash);
#endif
		invoice->setIsLoaded(TRUE);
		back.append(invoice);
	}
	return back;
}

void Invoice::create(PPSPerson *person) {
	QSettings settings;
	clear();
	setIsLoaded(false);

	setMemberUid(person->uid());
	setReference(createReference(person->uid()));
	setIssueDate(QDate::currentDate());
	setPayableDue(QDate::currentDate().addMonths(1));
	setPaidDate(QDate(1970, 1, 1));
	setReminded(0);
	setLastReminded(QDate(1970, 1, 1));
#ifndef FIO
	if (person->contributionClass() == PPSPerson::ContributeFull) {
		setAmount(settings.value("invoice/amount_default", 60).toFloat());
	} else {
		setAmount(settings.value("invoice/amount_limited", 30).toFloat());
	}
#else
	float a = setRecommendations(person->section());
	setAmount(a);
#endif
	setAmountPaid(0);
	setState(Invoice::StateOpen);
	switch (person->gender()) {
		case PPSPerson::GenderMale:
			setAddressPrefix(tr("Mr."));
			break;
		case PPSPerson::GenderFemale:
			setAddressPrefix(tr("Mrs."));
			break;
		default:
			setAddressPrefix("");
			break;
	}
	setAddressCompany("");
	setAddressName(person->givenName() + " " + person->familyName());
	setAddressStreet1(person->street());
	setAddressStreet2("");
	setAddressCity(person->postalCode() + " " + person->city());
	setAddressCountry(person->country());
	if (person->email().size() > 0) {
		setAddressEmail(person->email().first());
	}
	setForSection(person->section());
	setLanguage((Language)person->language());
	save();
}

#ifdef FIO
float Invoice::setRecommendations(QString section) {
	float back = 0.0;
	if (!l_recom.isEmpty()) {
		l_recom.clear();
	}
	Section sec(section);
	Section *s = sec.parent();
	if (s && s->loaded()) {
		back += setRecommendations(s->name());
	}
	l_recom.insert(sec.description().replace(QRegExp(";:"), ""), sec.amount());
	back += sec.amount();
	delete s;
	return back;
}
#endif

bool Invoice::pay(QDate *date) {
	return pay(amount(), date);
}

bool Invoice::pay(float amount, QDate *date) {
	QSettings settings;
	if (date == NULL) {
		setPaidDate(QDate::currentDate());
	} else {
		setPaidDate(QDate(date->year(), date->month(), date->day()));
	}
	setAmountPaid(amount);

#ifdef FIO
	float min_amount = settings.value("invoice/minimum_amount", 30.0).toFloat();
	if (amountPaid() >= min_amount) {
#else
	if (amountPaid() >= f_amount) {
#endif
		setState(StatePaid);
	}
	save();
	return _loaded;
}

XmlPdf *Invoice::createPdf(QString tpl) {
	if (tpl == NULL || tpl.isEmpty()) {
		tpl = QString("invoice");
	}
	
	QLocale locale;
	QSettings settings;
	XmlPdf *pdf = new XmlPdf;
	Section section(s_forSection);
	PPSPerson person;
	person.load(memberUid());
	QString lang = getLanguageString((Language)person.language());
	QString templateFile = settings.value(QString("pdf/%1_template").arg(tpl), "data/invoice_de.xml").toString();
	templateFile.replace(QRegExp("^(.*_)([a-zA-Z]{2})(\\.xml)$"), "\\1" + lang + "\\3");
	pdf->loadTemplate(templateFile);

	pdf->setVar("pp_country", settings.value("pdf/var_pp_country", "CH").toString());
	pdf->setVar("pp_zip", settings.value("pdf/var_pp_zip", "1337").toString());
	pdf->setVar("pp_city", settings.value("pdf/var_pp_city", "Vallorbe").toString());
	pdf->setVar("print_city", settings.value("pdf/var_print_city", "Vallorbe").toString());
	pdf->setVar("print_date", QDate::currentDate().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("print_date_extended", QDate::currentDate().toString( settings.value("pdf/date_format_extended", "dd. MMMM yyyy").toString() ));
	pdf->setVar("print_year", QDate::currentDate().toString("yyyy"));
	pdf->setVar("account_number", settings.value("pdf/bank_account_number", "01-84038-2").toString());

	pdf->setVar("invoice_reference", reference());
	pdf->setVar("invoice_number", reference());
	pdf->setVar("invoice_date", issueDate().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("invoice_payable_due", payableDue().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("invoice_esr", getEsr());
#ifndef FIO
	pdf->setVar("invoice_amount", locale.toString(amount(), 'f', 2));
	pdf->setVar("invoice_pay_amount", locale.toString(amount() - amountPaid(), 'f', 2));
#else
	pdf->setVar("invoice_amount", locale.toString(amount(), 'f', 2));
	pdf->setVar("invoice_pay_amount", locale.toString(amount() - amountPaid(), 'f', 2));
	
	// Add all recommendations from all Sections the user is in
	XmlPdfEntry *entry;
	QHash<QString, float>::const_iterator it = l_recom.constBegin();
	while (it != l_recom.constEnd()) {
		entry = pdf->addEntry("recommendations");
		entry->setVar("section", it.key());
		entry->setVar("amount", locale.toString(it.value(), 'f', 2));
		it++;
	}
	
	// Add the total as last entry
	entry = pdf->addEntry("recommendations");
	entry->setVar("section", tr("Total"));
	entry->setVar("amount", locale.toString(amount(), 'f', 2));
#endif
	
	pdf->setVar("member_number", QString::number(memberUid()));
	pdf->setVar("member_prefix", addressPrefix());
	pdf->setVar("member_company", addressCompany());
	pdf->setVar("member_name", addressName());
	pdf->setVar("member_street", addressStreet1());
	pdf->setVar("member_street2", addressStreet2());
	pdf->setVar("member_city", addressCity());
	pdf->setVar("member_country", addressCountry());
	pdf->setVar("member_email", addressEmail());
	pdf->setVar("member_section", forSection());
	pdf->setVar("member_nick", person.nickname());
	pdf->setVar("section_text", section.invoiceText(lang));
	pdf->setVar("section_logo", section.invoiceLogo());
	pdf->setVar("section_address", section.address());
	pdf->setVar("section_logo_file", section.logoIsFile() ? "1" : "");
	pdf->setVar("section_email", section.email());
	
	return pdf;
}

QString Invoice::getLanguageString(Language lang) {
	switch (lang) {
		case DE: return QString("de"); break;
		case FR: return QString("fr"); break;
		case IT: return QString("it"); break;
		case EN: return QString("en"); break;
	}
	return QString("en");
}

QString Invoice::getEsr() {
	QSettings settings;
	QString esr;
	if (settings.value("pdf/esr_currency", "CHF").toString() == "EUR") {
		esr.append("21"); // ESR in CHF: values must not match the boxes exactly
	} else {
		esr.append("01"); // ESR in CHF: values must not match the boxes exactly
	}
	esr.append( QString::number(esrChecksum(esr)) );
	esr.append(">"); // Checksum and seperator

	// Add the reference
	esr.append(reference().remove(" ")); // Append the Reference
	esr.append("+ "); // Seperator

	// Add the Account-Number
	QString account;
	QStringList accountNumber = settings.value("pdf/bank_account_number", "0-0-0").toString().split("-", QString::SkipEmptyParts);
	if (accountNumber.size() == 3) {
		account.append(accountNumber.at(0));
		account.append(QString(accountNumber.at(1)).prepend( QString("").fill(QChar('0'), 8 - accountNumber.at(1).length()) ));
		account.append(accountNumber.at(2));
	} else {
		account.append("000000000"); // No Account-Number
	}
	esr.append(account);
	//esr.append(QString::number( esrChecksum(account) )); // The last Char on the AccountNumber is the Checksum either
	esr.append(">"); // Seperator

	return esr;
}

int Invoice::esrChecksum(QString num) {
	int checkSum[10] = {0, 9, 4, 6, 8, 2, 7, 1, 3, 5};
	int pos, sum = 0;
	for (int i = 0; i < num.length(); i++) {
		pos = (sum += num.at(i).digitValue()) % 10;
		sum = checkSum[pos];
	}
	return (10 - sum) % 10;
}

void Invoice::setMemberUid(int memberUid) {
	i_memberUid = memberUid;
	emit memberUidChanged(memberUid);
}

void Invoice::setReference(QString reference) {
	s_reference = reference.remove(QRegExp("[^\\d]+"));
	emit referenceChanged(s_reference);
}

QString Invoice::createReference(int memberUid) {
	// Format: Random(2) Member-0-prepend(10) yyyy0 MMdd0 Random(4)
	//         xx0000000UIDyyyy0MMdd0XXXX
	// Formatted reference for a swiss credit slip (P = CheckDigit): 12 00000 01234 20120 05130 3415P
	QString ref(QString::number( (rand() % 10) + 10 )); // Random(2)
	QString uid = QString::number(memberUid);
	uid.prepend( QString("").fill(QChar('0'), 10-uid.size()) );
	ref.append(uid); // UID and '0' prepend to 10 chars
	ref.append(QDate::currentDate().toString("yyyy0MMdd0")); // Current date
	ref.append(QString::number( (rand() % 1000) + 1000 )); // Random(4)
	return ref;
}

QString Invoice::reference() {
	QString ref = s_reference;
	ref.prepend( QString("").fill( QChar('0'), 26 - ref.length()) );

	ref.append(QString::number( esrChecksum(ref) )); // Append the Checksum

	ref.insert(2, " ");
	ref.insert(8, " ");
	ref.insert(14, " ");
	ref.insert(20, " ");
	ref.insert(26, " ");
	return ref;
};

void Invoice::setIssueDate(QDate issueDate) {
	d_issueDate = issueDate;
	emit issueDateChanged(issueDate);
}

void Invoice::setPayableDue(QDate payableDue) {
	d_payableDue = payableDue;
	emit payableDueChanged(payableDue);
}

void Invoice::setPaidDate(QDate paidDate) {
	d_paidDate = paidDate;
	emit paidDateChanged(paidDate);
}

void Invoice::setAmount(float amount) {
	f_amount = amount;
	emit amountChanged(amount);
}

void Invoice::setAmountPaid(float amountPaid) {
	f_amountPaid = amountPaid;
	emit amountPaidChanged(amountPaid);
}

void Invoice::setState(State state) {
	m_state = state;
	emit stateChanged(state);
}

void Invoice::setReminded(int reminded) {
	i_reminded = reminded;
	emit remindedChanged(reminded);
}

void Invoice::setLastReminded(QDate lastReminded) {
	d_last_reminded = lastReminded;
	emit lastRemindedChanged(lastReminded);
}

void Invoice::setAddressPrefix(QString addressPrefix) {
	s_addressPrefix = addressPrefix;
	emit addressPrefixChanged(addressPrefix);
}

void Invoice::setAddressCompany(QString addressCompany) {
	s_addressCompany = addressCompany;
	emit addressCompanyChanged(addressCompany);
}

void Invoice::setAddressName(QString addressName) {
	s_addressName = addressName;
	emit addressNameChanged(addressName);
}

void Invoice::setAddressStreet1(QString addressStreet1) {
	s_addressStreet1 = addressStreet1;
	emit addressStreet1Changed(addressStreet1);
}

void Invoice::setAddressStreet2(QString addressStreet2) {
	s_addressStreet2 = addressStreet2;
	emit addressStreet2Changed(addressStreet2);
}

void Invoice::setAddressCity(QString addressCity) {
	s_addressCity = addressCity;
	emit addressCityChanged(addressCity);
}

void Invoice::setAddressCountry(QString addressCountry) {
	s_addressCountry = addressCountry;
	emit addressCountryChanged(addressCountry);
}

void Invoice::setAddressEmail(QString addressEmail) {
	s_addressEmail = addressEmail;
	emit addressEmailChanged(addressEmail);
}

void Invoice::setForSection(QString forSection) {
	s_forSection = forSection;
	emit forSectionChanged(forSection);
}

void Invoice::setLanguage(Language language) {
	m_language = language;
	emit languageChanged(language);
}

#ifdef FIO
void Invoice::setRecommendations(QHash<QString, float> recommendations) {
	l_recom.clear();
	QHash<QString, float>::iterator it = recommendations.begin();
	while (it != recommendations.end()) {
		l_recom.insert(it.key(), it.value());
		it++;
	}
	emit recommendationsChanged(recommendations);
}
#endif
