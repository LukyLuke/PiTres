/*
	The DonaitionWizard is for importing and adding Donations
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

#ifndef DonationWizard_H
#define DonationWizard_H

#include "ui_donation_import.h"
#include "data/Donation.h"
#include "data/Section.h"
#include "data/Person.h"

#include <cstdlib>
#include <cstdio>

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QSettings>
#include <QString>
#include <QCompleter>
#include <QFileDialog>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <QList>
#include <QHash>
#include <QMap>
#include <QDebug>

class DonationWizard : public QDialog, private Ui::DonationImportForm {
Q_OBJECT

public:
	DonationWizard(QWidget *parent = 0);
	virtual ~DonationWizard();

private slots:
	void wizardBack();
	void wizardNext();
	void showFileDialog();
	void reloadImport();
	
private:
	QMap<QString, QString> loadedSections;
	QList<QString> sectionsList;
	
	void enableWizardButtons();
	void wizardPrepareRecord();
	void wizardImport();
	void createSectionMapping();
	void previewImport();
};

#endif // DonationWizard_H
