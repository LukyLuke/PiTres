/*
	Import Members from configured LDAP-Connection into the Database
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

#include "LDAPImport.h"
#include "data/Person.h"

#include <vector>
#include <QSizePolicy>
#include <QWidget>
#include <QByteArray>
#include <QSettings>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>
#include <QList>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QDebug>

extern "C" {
#include <ldap.h>
}

LDAPImport::LDAPImport(QWidget *parent) {
	setupUi(this);
	QSettings settings;
	
	// Initialize some components
	connected = FALSE;
	version = LDAP_VERSION3;
	stackedWidget->setCurrentIndex(0);
	
	// Load saved values
	editServer->setText( settings.value("ldap/server", "localhost").toString() );
	editPort->setValue( settings.value("ldap/port", 389).toInt() );
	editBindDN->setText( settings.value("ldap/binddn", "uniqueIdentifier=<Your UID here>,dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString() );
	editBaseDN->setText( settings.value("ldap/basedn", "dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString() );
	editPassword->setText( settings.value("ldap/password", "").toString() );
	editSearch->setText( settings.value("ldap/search", "(objectClass=ppsPerson)").toString() );
	sectionExtract->setText( settings.value("ldap/sectionregex", "^.*,st=([a-z]{2}).*$|^.*,dc=(members).*$").toString() );
	
	// Setup StackWidget Navigation
	connect(btnGoToSearch, SIGNAL(clicked()), this, SLOT(nextPage()));
	connect(btnGoToImport, SIGNAL(clicked()), this, SLOT(nextPage()));
	connect(btnGoToSettings, SIGNAL(clicked()), this, SLOT(previousPage()));
	connect(btnEndBack, SIGNAL(clicked()), this, SLOT(previousPage()));
	
	// Connect Search and TestConnection
	connect(btnTestConnection, SIGNAL(clicked()), this, SLOT(testConnection()));
	connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchLdap()));
	connect(btnGoToImport, SIGNAL(clicked()), this, SLOT(importFromLdap()));
	connect(btnSaveData, SIGNAL(clicked()), this, SLOT(saveConfiguration()));
	connect(btnEmptyDatabase, SIGNAL(clicked()), this, SLOT(emptyDatabase()));
}

LDAPImport::~LDAPImport() {
	disconnectLdap();
}

void LDAPImport::nextPage() {
	if (stackedWidget->currentIndex() < (stackedWidget->count()-1)) {
		stackedWidget->setCurrentIndex(stackedWidget->currentIndex()+1);
	}
}
void LDAPImport::previousPage() {
	if (stackedWidget->currentIndex() >= 0) {
		stackedWidget->setCurrentIndex(stackedWidget->currentIndex()-1);
	}
}

void LDAPImport::saveConfiguration() {
	QSettings settings;
	settings.setValue("ldap/server", editServer->text());
	settings.setValue("ldap/port", editPort->value());
	settings.setValue("ldap/binddn", editBindDN->text());
	settings.setValue("ldap/basedn", editBaseDN->text());
	settings.setValue("ldap/password", editPassword->text());
	settings.setValue("ldap/search", editSearch->text());
	settings.setValue("ldap/sectionregex", sectionExtract->text());
}

void LDAPImport::disconnectLdap() {
	if (connected) {
		ldap_unbind_ext(ldap, NULL, NULL);
	}
	connected = FALSE;
}

void LDAPImport::connectLdap() {
	if (connected) return;

	int rc;
	QString host = "ldap://" + editServer->text() + ":" + QString::number(editPort->value());
	qDebug() << "Trying to connect: " + host;
	
	if (LDAP_SUCCESS == ldap_initialize(&ldap, host.toStdString().c_str())) {
		qDebug() << "Connection initialized...";
		
		if (ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_OPT_SUCCESS) {
			qDebug() << "Could not set LDAP_OPT_PROTOCOL_VERSION to v3";
		}
		if (ldap_set_option(ldap, LDAP_OPT_DEREF, LDAP_DEREF_NEVER) != LDAP_OPT_SUCCESS) {
			qDebug() << QString("Could not set LDAP_OPT_DEREF to NEVER");
		}
		if (ldap_set_option(ldap, LDAP_OPT_REFERRALS, LDAP_OPT_ON) != LDAP_OPT_SUCCESS) {
			qDebug() << "Could not set LDAP_OPT_REFERRALS off.";
		}
		
		QByteArray pwd = editPassword->text().toLocal8Bit();
		cred.bv_val = pwd.data();
		cred.bv_len = strlen(cred.bv_val);
		
		QByteArray binddn = editBindDN->text().toLocal8Bit();
		rc = ldap_sasl_bind_s(ldap, binddn.data(), LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
		if (rc != LDAP_SUCCESS) {
			ldapError = ldap_err2string(rc);
			qDebug() << QString("Unable to bind(%1): %2").arg(rc).arg(ldapError);
			ldap_unbind_ext(ldap, NULL, NULL);
			return;
		}
		qDebug() << "Bound to LDAP-Server with BindDN: " + binddn;
		connected = TRUE;
	}
}

void LDAPImport::testConnection() {
	disconnectLdap();
	connectLdap();
	labelConnectionState->setText("<span style=\"font-weight:600;" + (connected ? "color:#008000;\">"+tr("Connection Established") : "color:#c00000;\">"+tr("Unable to connect: %1").arg(ldapError)) + "</span>");
}

void LDAPImport::searchLdap() {
	QMessageBox::information(this,
		tr("PiTres LDAP-Import"),
		tr("The Search-Function does not work in this version. Go forward and import the Entries...")
	);
}

void LDAPImport::importFromLdap() {
	connectLdap();
	if (!connected) {
		QMessageBox::critical(this,
			tr("PiTres LDAP-Import"),
			tr("Unable to connect to the LDAP-Server %1 with BindDN %2.\n\n"
			   "Please go back, check your Settings and try them with 'Test Connection'.").arg(editServer->text()).arg(editBindDN->text())
		);
		return;
	}
	
	int rc;
	QByteArray basedn = editBaseDN->text().toLocal8Bit();
	QByteArray filter = editSearch->text().toLocal8Bit();
	QByteArray _attrs = editAttributes->text().simplified().toLocal8Bit();
	QList<QByteArray> attrlist = _attrs.split(',');
	int i, attrc = attrlist.length();
	char **attrs = new char*[attrc];
	for (i = 0; i < attrc; i++) {
		attrs[i] = attrlist[i].data();
	}
	
	LDAPMessage *res = NULL, *msg = NULL;
	BerElement *ber = NULL;
	//                                                                             attrs
	// If we add "attrs" we get a Segfault here - strange...
	rc = ldap_search_ext_s(ldap, basedn.data(), LDAP_SCOPE_SUBTREE, filter.data(), NULL, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
	
	// Cleanup calling parameters
	delete[] attrs;
	
	if (rc != LDAP_SUCCESS) {
		ldapError = ldap_err2string(rc);
		qDebug() << QString("Search failed(%1): %2").arg(rc).arg(ldapError);
		if (res != NULL) {
			ldap_msgfree(res);
		}
		return;
	}
	qDebug() << QString("Search for %1 succeed").arg(editSearch->text());
	
	// Create Persons Database Tables if needed
	PPSPerson::createTables();
	
	// Prepare the Progress-Bar
	int max = ldap_count_entries(ldap, res);
	int count = 0;
	progressBar->setMinimum(1);
	progressBar->setMaximum(max);
	progressBar->setValue(0);
	
	// Get all results
	char *attribute;
	berval **values, dnval;
	PPSPerson *person;
	QStringList dnlist;
	QString dn;

	for (msg = ldap_first_entry(ldap, res); msg != NULL; msg = ldap_next_entry(ldap, msg)) {
		ldap_get_dn_ber(ldap, msg, &ber, &dnval);
		
		progressBar->setValue(++count);
		labelImportState->setText(QString("Import %1 of %2").arg(count).arg(max));
		
		person = new PPSPerson;
		
		setPersonValue(person, QString("section"), "");
		dnlist = sectionExtract->text().split('|');
		for (int i = 0; i < dnlist.size(); i++) {
			dn = QString(dnval.bv_val);
			dn.replace(QRegExp(dnlist.at(i)), "\\1");
			if ((dn != dnval.bv_val)) {
				setPersonValue(person, QString("section"), dn);
				break;
			}
		}
		
		// dont't insert empty sections
		if (person->section() == "") {
			continue;
		}
		
		for (attribute = ldap_first_attribute(ldap, msg, &ber); attribute != NULL; attribute = ldap_next_attribute(ldap, msg, ber)) {
			if ((values = ldap_get_values_len(ldap, msg, attribute)) != NULL) {
				for (int i = 0; values[i] != NULL; i++) {
					setPersonValue(person, QString::fromUtf8(attribute), QString::fromUtf8(values[i]->bv_val));
				}
			}
			ldap_value_free_len(values);
		}
		ldap_memfree(attribute);
		
		person->save();
		person->clear();
	}
	
	// Clean up
	if (ber != NULL) {
		ber_free(ber, 0);
	}
	if (msg != NULL) {
		ldap_msgfree(msg);
	}
	if (res != NULL) {
		ldap_msgfree(res);
	}
	if (person != NULL) {
		delete person;
	}
}

void LDAPImport::emptyDatabase() {
	PPSPerson::emptyTables();
}

void LDAPImport::setPersonValue(PPSPerson *person, QString field, QString value) {
	field = field.toLower();
	
	if (field == "uniqueidentifier") {
		person->setUid(value.toInt());
		
	} else if (field == "ppscontributionclass") {
		person->setContributionClass(value.toInt() == PPSPerson::ContributeFull ? PPSPerson::ContributeFull : PPSPerson::ContributeStudent);
		
	} else if (field == "uid") {
		person->setNickname(value);
		
	} else if (field == "section") {
		person->setSection(value);
		
	} else if (field == "ppsgender") {
		switch(value.toInt()) {
			case 0: person->setGender(PPSPerson::GenderUnknown); break;
			case 1: person->setGender(PPSPerson::GenderMale); break;
			case 2: person->setGender(PPSPerson::GenderFemale); break;
			case 3: person->setGender(PPSPerson::GenderBoth); break;
		}
		
	} else if (field == "sn") {
		person->setFamilyName(value);
		
	} else if (field == "givenname") {
		person->setGivenName(value);
		
	} else if (field == "street") {
		person->setStreet(value);
		
	} else if (field == "postalcode") {
		person->setPostalCode(value);
		
	} else if (field == "l") {
		person->setCity(value);
		
	} else if (field == "c") {
		person->setCountry(value);
		
	} else if (field == "st") {
		person->setState(value);
		
	} else if (field == "telephonenumber") {
		person->addTelephone(value);
		
	} else if (field == "mobile") {
		person->addMobile(value);
		
	} else if (field == "mail") {
		person->addEmail(value);
		
	} else if (field == "ppsbirthdate") {
		value.resize(8);
		person->setBirthdate(QDate::fromString(value, "yyyyMMdd"));
		
	} else if (field == "ppsjoining") {
		value.resize(8);
		person->setJoining(QDate::fromString(value, "yyyyMMdd"));
		
	} else if (field == "ppsvotingrightuntil") {
		value.resize(8);
		person->setLdapPaidDue(QDate::fromString(value, "yyyyMMdd"));
		
	} else if (field == "preferredlanguage") {
		value = value.toLower();
		if (value == "de") { person->setLanguage(PPSPerson::DE); }
		else if (value == "it") { person->setLanguage(PPSPerson::IT); }
		else if (value == "fr") { person->setLanguage(PPSPerson::FR); }
		else { person->setLanguage(PPSPerson::EN); }
		
	} else if (field == "employeetype") {
		switch (value.toInt()) {
			case PPSPerson::LandLubber: person->setType(PPSPerson::LandLubber); break;
			case PPSPerson::Sympathizer: person->setType(PPSPerson::Sympathizer); break;
			case PPSPerson::Pirate: person->setType(PPSPerson::Pirate); break;
			case PPSPerson::Veteran: person->setType(PPSPerson::Veteran); break;
			case PPSPerson::PlankWalker: person->setType(PPSPerson::PlankWalker); break;
			case PPSPerson::FleetPirate: person->setType(PPSPerson::FleetPirate); break;
			default: person->setType(PPSPerson::UnknownType); break;
		}
	}
}
