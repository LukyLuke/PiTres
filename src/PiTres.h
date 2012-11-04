/*
	The main Application - PiTres
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

#ifndef PiTres_H
#define PiTres_H

#include "ui_PiTres.h"
#include "ui_settings.h"
#include "ui_invoicewizard.h"

#include "Userlist.h"
#include "SentBills.h"
#include "PaymentImport.h"
#include "LDAPImport.h"
#include "InvoiceWizard.h"
#include "Contributions.h"
#include "BudgetView.h"
#include "SectionEdit.h"
#include "Donations.h"
#include "DonationWizard.h"
#include "Statistics.h"

#include "data/Person.h"
#include "data/Invoice.h"
#include "data/Section.h"
#include "data/BudgetEntity.h"
#include "data/Donation.h"
#include "helper/XmlPdf.h"

#include <QtGui/QMainWindow>
#include <QtGui>
#include <QFileDialog>
#include <QtGui/QGridLayout>
#include <QFileInfo>
#include <QSettings>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QSqlDatabase>

class PiTres : public QMainWindow, private Ui::MainWindow {
Q_OBJECT
private:
	QDialog *settingsDialog;
	Ui::settingsForm settingsForm;
	QSqlDatabase db;
	QWidget *content;
	void setContent(QWidget* widget);

protected:
	void connectActions();
	void debugAction(QString sender);

public:
	PiTres(QMainWindow *parent = 0);
	virtual ~PiTres();
	void showStatusMessage(QString msg);

public slots:
	void importFromLDAP();
	void showAbout();
	void showHelp(int page = 0);
	void showUsers();
	void showSentBills();
	void showImportPayments();
	void showInvoiceWizard();
	void showSettings();
	void doSaveSettings();
	void showInvoiceFileDialog();
	void showReminderFileDialog();
	void showReceiptFileDialog();
	void showStatisticFileDialog();
	void showContributions();
	void showBudget();
	void showSectionEdit();
	void showDonations();
	void showDonationWizard();
	void showStatistics();
};

#endif // PiTres_H
