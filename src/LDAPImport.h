#ifndef LDAPImport_H
#define LDAPImport_H

#include "../ui_LDAPImport.h"

#include "data/Person.h"

#include <QWidget>
#include <QString>
#include <QtSql>

extern "C" {
#include <ldap.h>
}

class LDAPImport : public QWidget, private Ui::LDAPImportForm {
Q_OBJECT
private:
	LDAP *ldap;
	QString ldapError;
	int version;
	struct berval cred;
	bool connected;
	QSqlDatabase db;
	
	void connectLdap();
	void disconnectLdap();
	void openDatabase();
	void setPersonValue(PPSPerson *person, QString field, QString value);

public:
	LDAPImport(QWidget *parent = 0);
	virtual ~LDAPImport();

public slots:
	void nextPage();
	void previousPage();
	void testConnection();
	void searchLdap();
	void importFromLdap();
	void saveConfiguration();
	void emptyDatabase();

};

#endif // LDAPImport_H 
