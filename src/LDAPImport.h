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

#ifndef LDAPImport_H
#define LDAPImport_H

#include "../ui_LDAPImport.h"

#include "data/Person.h"
#include "data/Section.h"

#include <QWidget>
#include <QString>
#include <QtSql>

#ifndef WIN32
extern "C" {
#include <ldap.h>
}
#endif

class LDAPImport : public QWidget, private Ui::LDAPImportForm {
Q_OBJECT
private:
#ifndef WIN32
	LDAP *ldap;
  struct berval cred;
#endif
	QString ldapError;
	int version;
	bool connected;
	QSqlDatabase db;
	
	void connectLdap();
	void disconnectLdap();
	void setPersonValue(PPSPerson *person, QString field, QString value);
	void addSection(QString section);

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
