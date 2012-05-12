
#include "Invoice.h"

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QVariant>
#include <QDate>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

Invoice::Invoice(QObject *parent) : QObject(parent) {
	clear();

	// Staticly defined values for ESR-Modulo-10 Checksum
	esrChecksumList.append("09468271350");
	esrChecksumList.append("94682713509");
	esrChecksumList.append("46827135098");
	esrChecksumList.append("68271350947");
	esrChecksumList.append("82713509466");
	esrChecksumList.append("27135094685");
	esrChecksumList.append("71350946824");
	esrChecksumList.append("13509468273");
	esrChecksumList.append("35094682712");
	esrChecksumList.append("50946827131");
}

Invoice::~Invoice() {}

void Invoice::clear() {
	setIsLoaded(TRUE);
	i_memberUid = 0;
	s_reference = "";
	d_issueDate = QDate::currentDate();
	d_payableDue = QDate::currentDate().addDays(30);
	d_paidDate = QDate(1970, 0, 0);
	f_amount = 0.0;
	f_amountPaid = 0.0;
	m_state = StateOpen;
	i_reminded = 0;
	s_addressPrefix = "";
	s_addressCompany = "";
	s_addressName = "";
	s_addressStreet1 = "";
	s_addressStreet2 = "";
	s_addressCity = "";
	s_addressCountry = "";
	s_addressEmail = "";
	s_forSection = "";
}

void Invoice::setIsLoaded(bool loaded) {
	_loaded = loaded;
}

void Invoice::createTables(QSqlDatabase db) {
	QSqlQuery query(db);
	query.prepare("CREATE TABLE IF NOT EXISTS pps_invoice (member_uid INTEGER, reference TEXT, issue_date DATE, payable_date DATE, paid_date DATE, "
	              "amount FLOAT, amount_paid FLOAT, state INTEGER, address_prefix TEXT, address_company TEXT, address_name TEXT, address_street1 TEXT, "
	              "address_street2 TEXT, address_city TEXT, address_email TEXT, for_section TEXT);");
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
}

void Invoice::save(QSqlDatabase db) {
	QSqlQuery query(db);
	query.prepare(QString(_loaded ? "UPDATE" : "INSERT INTO") + " pps_invoice (member_uid,reference,issue_date,payable_date,paid_date,amount,amount_paid,state,"
	              "address_prefix,address_company,address_name,address_street1,address_street2,address_city,address_email,for_section) VALUES ("
	              ":member,:reference,:issued,:payable,:paid,:amount,:amount_paid,:state,:prefix,:company,:name,"
	              ":street1,:street2,:city,:email,:section)" + QString(_loaded ? " WHERE member_uid=:member" : "") + ";");

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
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
}

void Invoice::loadLast(QSqlDatabase db, int member) {
	clear();
	QSqlQuery query;
	query.prepare("SELECT * FROM pps_invoice WHERE member_uid=? ORDER BY issue_date DESC;");
	query.bindValue(0, member);
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
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
		s_addressPrefix = query.value(record.indexOf("address_prefix")).toString();
		s_addressCompany = query.value(record.indexOf("address_company")).toString();
		s_addressName = query.value(record.indexOf("address_name")).toString();
		s_addressStreet1 = query.value(record.indexOf("address_street1")).toString();
		s_addressStreet2 = query.value(record.indexOf("address_street2")).toString();
		s_addressCity = query.value(record.indexOf("address_city")).toString();
		s_addressEmail = query.value(record.indexOf("address_email")).toString();
		s_forSection = query.value(record.indexOf("for_section")).toString();
	}
}

QList<Invoice *> Invoice::getInvoicesForMember(QSqlDatabase db, int member) {
	QList<Invoice *> back;
	
	QSqlQuery query;
	query.prepare("SELECT * FROM pps_invoice WHERE id=? ORDER BY issue_date DESC;");
	query.bindValue(0, member);
	query.exec();
	QSqlRecord record = query.record();
	
	while(query.next()) {
		Invoice *invoice;
		invoice->setMemberUid(query.value(record.indexOf("member_uid")).toInt());
		invoice->setReference(query.value(record.indexOf("reference")).toString());
		invoice->setIssueDate(query.value(record.indexOf("issue_date")).toDate());
		invoice->setPayableDue(query.value(record.indexOf("payable_date")).toDate());
		invoice->setPaidDate(query.value(record.indexOf("paid_date")).toDate());
		invoice->setAmount(query.value(record.indexOf("amount")).toFloat());
		invoice->setAmountPaid(query.value(record.indexOf("amount_paid")).toFloat());
		invoice->setState((State) query.value(record.indexOf("state")).toInt());
		invoice->setAddressPrefix(query.value(record.indexOf("address_prefix")).toString());
		invoice->setAddressCompany(query.value(record.indexOf("address_company")).toString());
		invoice->setAddressName(query.value(record.indexOf("address_name")).toString());
		invoice->setAddressStreet1(query.value(record.indexOf("address_street1")).toString());
		invoice->setAddressStreet2(query.value(record.indexOf("address_street2")).toString());
		invoice->setAddressCity(query.value(record.indexOf("address_city")).toString());
		invoice->setAddressEmail(query.value(record.indexOf("address_email")).toString());
		invoice->setForSection(query.value(record.indexOf("for_section")).toString());
		invoice->setIsLoaded(TRUE);
		back.append(invoice);
	}
	return back;
}

XmlPdf *Invoice::createPdf() {
	QSettings settings;
	XmlPdf *pdf = new XmlPdf;
	pdf->loadTemplate( settings.value("pdf/invoice_template", "data/invoice.xml").toString() );
	return pdf;
}

QString Invoice::getEsr() {
	QSettings settings;
	QString esr;
	if (settings.value("pdf/esr_currency", "CHF").toString() == "EUR") {
		esr.append("21"); // ESR in CHF: values must not match the boxes exactly
	} else {
		esr.append("01"); // ESR in CHF: values must not match the boxes exactly
	}
	esr.append(esrChecksum(esr));
	esr.append(">"); // Checksum and seperator
	
	// Add the reference
	/*QString _ref = reference().remove(QRegExp("\\s+"));
	_ref.prepend(QString("").fill(QChar('0'), 26-_ref.length()));
	esr.append(_ref); // Append the Reference
	esr.append( esrChecksum(_ref) ); // Append the Checksum
	*/
	esr.append(reference().remove(" ")); // Append the Reference
	esr.append("+ "); // Seperator
	
	// Add the Account-Number
	QString account;
	QStringList accountNumber = settings.value("pdf/invoice_account_number", "0-0-0").toString().split("-", QString::SkipEmptyParts); 
	if (accountNumber.size() == 3) {
		account.append(accountNumber.at(0));
		account.append(QString(accountNumber.at(1)).prepend( QString("").fill(QChar('0'), 8 - accountNumber.at(1).length()) ));
		account.append(accountNumber.at(2));
	} else {
		account.append("000000000"); // No Account-Number
	}
	esr.append(account);
	//esr.append(esrChecksum(account)); // The last Char on the AccountNumber is the Checksum either
	esr.append(">"); // Seperator
	
	return esr;
}

QString Invoice::esrChecksum(QString num) {
	QString sum("0");
	QString::const_iterator it = num.constBegin();
	for (int i = 0; i < num.length(); i++) {
		sum = esrChecksumList.at(sum.toInt()).at(num.at(i).digitValue());
		it++;
	}
	return esrChecksumList.at(sum.toInt()).at(10);
}

void Invoice::setMemberUid(int memberUid) {
	i_memberUid = memberUid;
	emit memberUidChanged(memberUid);
}

void Invoice::setReference(QString reference) {
	s_reference = reference.remove(QRegExp("[^\\d]+"));
	emit referenceChanged(s_reference);
}

QString Invoice::reference() {
	QString ref = s_reference;
	ref.prepend( QString("").fill( QChar('0'), 26 - ref.length()) );
	
	QString sum = esrChecksum(ref);
	ref.append(sum); // Append the Checksum
	
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

