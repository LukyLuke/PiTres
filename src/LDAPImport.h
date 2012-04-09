#ifndef LDAPImport_H
#define LDAPImport_H

#include "../ui_LDAPImport.h"

#include <QWidget>

extern "C" {
#include <ldap.h>
}

class LDAPImport : public QWidget, private Ui::LDAPImportForm {
Q_OBJECT
private:
	LDAP *ldap;
	QString ldapError;
	bool connected;
	void connectLdap();
	void disconnectLdap();

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

};

#endif // LDAPImport_H 
