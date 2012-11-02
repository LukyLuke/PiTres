/*
	The Donaitions Widget is for managing all Donaitions
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

#ifndef Donations_H
#define Donations_H

#include "ui_donations.h"
#include "data/Donation.h"
#include "data/Section.h"
#include "data/Person.h"
#include "helper/Smtp.h"

#include <cstdlib>
#include <cstdio>

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QCompleter>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QList>
#include <QSettings>
#include <QHash>
#include <QFileDialog>
#include <QDebug>

class Donations : public QWidget, private Ui::DonationsForm {
Q_OBJECT

public:
	Donations(QWidget *parent = 0);
	virtual ~Donations();

private slots:
	void searchData();
	void exportData();
	void sendEmail();
	void selectSection();
	void selectYear();
	void searchDonations();
	void wizardBack();
	void wizardNext();
	void showFileDialog();
	void reloadImport();
	
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	QSqlQueryModel *donationsModel;
	QHash<QString, QString> sectionQif;
	bool fileImport;
	
	QSqlQuery createQuery();
	void loadData();
	void showOverview();
	void createQif();
	void loadSectionDonations();
	QSqlQuery createDonationsQuery();
	void showDonationsDetails();
	void enableWizardButtons();
	void wizardPrepareImport();
	void wizardImport();
};

#endif // Donations_H
