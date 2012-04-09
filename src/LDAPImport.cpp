
#include "LDAPImport.h"

#include <vector>
#include <QSizePolicy>
#include <QWidget>
#include <QByteArray>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>

extern "C" {
#include <ldap.h>
}


LDAPImport::LDAPImport(QWidget *parent) {
	setupUi(this);
	
	// Initialize some components
	connected = FALSE;
	stackedWidget->setCurrentIndex(0);
	
	// Load saved values
	QSettings settings;
	editServer->setText( settings.value("ldap/server", editServer->text()).toString() );
	editPort->setText( settings.value("ldap/port", editPort->text()).toString() );
	editBindDN->setText( settings.value("ldap/binddn", editBindDN->text()).toString() );
	editBaseDN->setText( settings.value("ldap/basedn", editBaseDN->text()).toString() );
	editPassword->setText( settings.value("ldap/password", editPassword->text()).toString() );
	
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
	settings.setValue("ldap/port", editPort->text());
	settings.setValue("ldap/binddn", editBindDN->text());
	settings.setValue("ldap/basedn", editBaseDN->text());
	settings.setValue("ldap/password", editPassword->text());
}

void LDAPImport::disconnectLdap() {
	if (connected) {
		ldap_unbind_ext(ldap, NULL, NULL);
	}
	connected = FALSE;
}

void LDAPImport::connectLdap() {
	if (connected) return;
	
	QString host = "ldap://" + editServer->text() + ":" + editPort->text();
	qDebug() << "Trying to connect: " + host;
	
	if (LDAP_SUCCESS == ldap_initialize(&ldap, host.toStdString().c_str())) {
		qDebug() << "Connection initialized...";
		
		int rc, version = LDAP_VERSION3;
		struct berval cred;
		
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
	connectLdap();
	if (!connected) {
		QMessageBox::critical(this,
			tr("PiTres LDAP-Import"),
			tr("Unable to connect to the LDAP-Server %1 with BindDN %2.\n\n"
			   "Please go back, check your Settings and try them with 'Test Connection'.").arg(editServer->text()).arg(editBindDN->text())
		);
		return;
	}
	
	QByteArray basedn = editBaseDN->text().toLocal8Bit();
	QByteArray filter = editSearch->text().toLocal8Bit();
	QByteArray _attrs = editAttributes->text().toLocal8Bit();
	QList<QByteArray> attrlist = _attrs.split(',');
	int attrc = attrlist.length();
	char **attrs = new char*[attrc];
	for (int i = 0; i < attrc; i++) {
		attrs[i] = attrlist[i].data();
	}
	
	int rc;
	LDAPMessage *res = NULL, *msg = NULL;
	BerElement *ber = NULL;
	char *attribute;
	berval **values; //                                                            attrs
	rc = ldap_search_ext_s(ldap, basedn.data(), LDAP_SCOPE_SUBTREE, filter.data(), NULL, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, &res);
	
	// Cleanup calling parameters
	delete [] attrs;
	
	if (rc != LDAP_SUCCESS) {
		ldapError = ldap_err2string(rc);
		qDebug() << QString("Search failed(%1): %2").arg(rc).arg(ldapError);
		return;
	}
	qDebug() << QString("Search for %1 succed on %2").arg(editSearch->text()).arg(editAttributes->text());
	
	// Get al results
	for (msg = ldap_first_entry(ldap, res); msg != NULL; msg = ldap_next_entry(ldap, msg)) {
		//qDebug() << "\nEntry:";
		for (attribute = ldap_first_attribute(ldap, msg, &ber); attribute != NULL; attribute = ldap_next_attribute(ldap, msg, ber)) {
			if ((values = ldap_get_values_len(ldap, msg, attribute)) != NULL) {
				for (int i = 0; values[i] != NULL; i++) {
					//qDebug() << QString("%1: %2").arg(attribute).arg(values[i]->bv_val);
				}
			}
			ldap_value_free_len(values);
		}
		ldap_memfree(attribute);
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
}

void LDAPImport::importFromLdap() {}

//void LDAPImport::loadData() {
	// Fields: uniqueidentifier,uid,sn,givenname,street,postalcode,l,c,st,telephonenumber.mobile,ppsgender,ppscontributionclass,mail,employeetype,ppsbirthdate,ppsjoining,preferredlanguage,jpegphoto
	// Search: (objectClass=ppsPerson)
	// Base: dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch
	// Bind: uniqueIdentifier=<Your UID here>,dc=members,st=ts,dc=piratenpartei,dc=ch
//}
 
