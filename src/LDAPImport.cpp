
#include "LDAPImport.h"

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
	openDatabase();
	
	// Load saved values
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
	db.close();
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

	int rc;
	QString host = "ldap://" + editServer->text() + ":" + editPort->text();
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
	qDebug() << QString("Search for %1 succed on %2").arg(editSearch->text()).arg(editAttributes->text());
	
	// Get al results
	char *attribute;
	berval **values, dnval;
	QSqlQuery *query;
	QString dn;
	int mail = 0;
	for (msg = ldap_first_entry(ldap, res); msg != NULL; msg = ldap_next_entry(ldap, msg)) {
		ldap_get_dn_ber(ldap, msg, &ber, &dnval);
		
		query = prepareQuery();
		dn = QString(dnval.bv_val);
		dn.replace(sectionExtract->text(), "\\1");
		bindQueryValue(query, QString("section"), dn);
		mail = 0;
		
		for (attribute = ldap_first_attribute(ldap, msg, &ber); attribute != NULL; attribute = ldap_next_attribute(ldap, msg, ber)) {
			if ((values = ldap_get_values_len(ldap, msg, attribute)) != NULL) {
				for (int i = 0; values[i] != NULL; i++) {
					if (QString(attribute) == "mail") {
						bindQueryValue(query, QString("mail"+mail), QString(values[i]->bv_val));
						mail++;
					} else {
						bindQueryValue(query, QString(attribute), QString(values[i]->bv_val));
					}
				}
			}
			ldap_value_free_len(values);
		}
		ldap_memfree(attribute);
		
		bindUnboundQueryValues(query);
		query->exec();
		qDebug() << query->lastError();
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
	if (query != NULL) {
		delete query;
	}
}

void LDAPImport::importFromLdap() {}

void LDAPImport::openDatabase() {
	QSettings settings;
	QFileInfo dbfile(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	
	// Load the Database
	if (!dbfile.absoluteDir().exists()) {
		dbfile.absoluteDir().mkpath(dbfile.absolutePath());
		qDebug() << "Path did not exists, created: " + dbfile.absolutePath();
	}
	
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbfile.absoluteFilePath());
	db.open();
}

void LDAPImport::createTmpTables() {
	QSqlQuery query("CREATE TEMPORARY TABLE ldap_persons (uid INTEGER, type INTEGER, nickname TEXT, gender TEXT, surname TEXT, "
	                "lastname TEXT, address TEXT, plz TEXT, city TEXT, country TEXT, state TEXT, telephone TEXT, mobile TEXT, "
	                "mail1 TEXT,  mail2 TEXT,  mail3 TEXT, birthday DATE, language TEXT, joining DATE);", db);
}

QSqlQuery *LDAPImport::prepareQuery() {
	QSqlQuery *query = new QSqlQuery(db);
	query->prepare("INSERT INTO ldap_persons (uid, type, nickname, gender, surname, lastname, address, plz, city, country, state, "
	               "telephone, mobile, mail1, mail2, mail3, birthday, language, joining) "
	               "VALUES (:uid, :type, :nickname, :gender, :surname, :lastname, :address, :plz, :city, :country, :state, "
	               ":telephone, :mobile, :mail1, :mail2, :mail3, :birthday, :language, :joining);");
	return query;
}

void LDAPImport::bindQueryValue(QSqlQuery *query, QString field, QString value) {
	field = field.toLower();
	
	if (field == "uniqueidentifier") {
		query->bindValue(":uid", value);
	} else if (field == "ppscontributionclass") {
		query->bindValue(":type", value);
	} else if (field == "uid") {
		query->bindValue(":nickname", value);
	} else if (field == "ppsgender") {
		query->bindValue(":gender", value);
	} else if (field == "sn") {
		query->bindValue(":surname", value);
	} else if (field == "givenname") {
		query->bindValue(":lastname", value);
	} else if (field == "street") {
		query->bindValue(":address", value);
	} else if (field == "postalcode") {
		query->bindValue(":plz", value);
	} else if (field == "l") {
		query->bindValue(":city", value);
	} else if (field == "c") {
		query->bindValue(":country", value);
	} else if (field == "state") {
		query->bindValue(":state", value);
	} else if (field == "telephonenumber") {
		query->bindValue(":telephone", value);
	} else if (field == "mobile") {
		query->bindValue(":mobile", value);
	} else if (field == "mail1") {
		query->bindValue(":mail1", value);
	} else if (field == "mail2") {
		query->bindValue(":mail2", value);
	} else if (field == "mail3") {
		query->bindValue(":mail3", value);
	} else if (field == "ppsbirthdate") {
		query->bindValue(":birthday", value);
	} else if (field == "ppsjoining") {
		query->bindValue(":joining", value);
	} else if (field == "preferredlanguage") {
		query->bindValue(":language", value);
	}
}

void LDAPImport::bindUnboundQueryValues(QSqlQuery *query) {
	QMap<QString, QVariant> map = query->boundValues();
	QList<QString> keys = map.keys();
	
	if (keys.indexOf("uid") == -1) {
		bindQueryValue(query, "uniqueidentifier", "");
	} else if (keys.indexOf("type") == -1) {
		bindQueryValue(query, "ppscontributionclass", "");
	} else if (keys.indexOf("nickname") == -1) {
		bindQueryValue(query, "uid", "");
	} else if (keys.indexOf("gender") == -1) {
		bindQueryValue(query, "ppsgender", "");
	} else if (keys.indexOf("surname") == -1) {
		bindQueryValue(query, "sn", "");
	} else if (keys.indexOf("lastname") == -1) {
		bindQueryValue(query, "givenname", "");
	} else if (keys.indexOf("address") == -1) {
		bindQueryValue(query, "street", "");
	} else if (keys.indexOf("plz") == -1) {
		bindQueryValue(query, "postalcode", "");
	} else if (keys.indexOf("city") == -1) {
		bindQueryValue(query, "l", "");
	} else if (keys.indexOf("country") == -1) {
		bindQueryValue(query, "c", "");
	} else if (keys.indexOf("state") == -1) {
		bindQueryValue(query, "state", "");
	} else if (keys.indexOf("telephone") == -1) {
		bindQueryValue(query, "telephonenumber", "");
	} else if (keys.indexOf("mobile") == -1) {
		bindQueryValue(query, "mobile", "");
	} else if (keys.indexOf("mail1") == -1) {
		bindQueryValue(query, "mail1", "");
	} else if (keys.indexOf("mail2") == -1) {
		bindQueryValue(query, "mail2", "");
	} else if (keys.indexOf("mail3") == -1) {
		bindQueryValue(query, "mail3", "");
	} else if (keys.indexOf("birthday") == -1) {
		bindQueryValue(query, "ppsbirthdate", "");
	} else if (keys.indexOf("joining") == -1) {
		bindQueryValue(query, "ppsjoining", "");
	} else if (keys.indexOf("language") == -1) {
		bindQueryValue(query, "preferredlanguage", "");
	}
}
