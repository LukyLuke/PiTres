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

#include "PiTres.h"
#include "Userlist.h"
#include "SentBills.h"
#include "PaymentImport.h"
#include "LDAPImport.h"
#include "InvoiceWizard.h"
#include "Contributions.h"
#include "BudgetView.h"

#include "data/Person.h"
#include "data/Invoice.h"
#include "data/Section.h"
#include "data/BudgetEntity.h"
#include "helper/XmlPdf.h"

#include <QtGui>
#include <QFileDialog>
#include <QtGui/QGridLayout>
#include <QFileInfo>
#include <QSettings>

PiTres::PiTres(QMainWindow *parent) : QMainWindow(parent) {
	QSettings settings;
	setupUi(this);
	
	QLocale locale(settings.value("core/locale", "de_CH").toString());
	QLocale::setDefault(locale);
	
	// Resize
	resize(settings.value("core/size", QSize(800, 600)).toSize());
	if (settings.value("core/maximized", false).toBool()) {
		showMaximized();
	} else if (settings.value("core/fullscreen", false).toBool()) {
		showFullScreen();
	}
	
	content = new QWidget();
	connectActions();
	
	settingsDialog = new QDialog(this);
	settingsForm.setupUi(settingsDialog);
	connect(settingsForm.actionSave, SIGNAL(triggered()), this, SLOT(doSaveSettings()));
	
	connect(settingsForm.invoiceSelect, SIGNAL(clicked()), this, SLOT(showInvoiceFileDialog()));
	connect(settingsForm.reminderSelect, SIGNAL(clicked()), this, SLOT(showReminderFileDialog()));
	connect(settingsForm.receiptSelect, SIGNAL(clicked()), this, SLOT(showReceiptFileDialog()));
	
	// Load the Database
	QFileInfo dbfile(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	if (!dbfile.absoluteDir().exists()) {
		dbfile.absoluteDir().mkpath(dbfile.absolutePath());
		qDebug() << "Path did not exists, created: " + dbfile.absolutePath();
	}
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbfile.absoluteFilePath());
	db.open();
	
	// Create Persons Database Tables if needed
	PPSPerson::createTables();
	Invoice::createTables();
	Section::createTables();
	BudgetView::createTables();
	BudgetEntity::createTables();
	
	// Set some default values we need later
	settings.setValue("sentbills/sincedate", QDate::currentDate().addMonths(-3));
	settings.setValue("sentbills/maxdate", QDate::currentDate().addDays(1));
}

PiTres::~PiTres() {
	delete content;
	
	// Store current Window-Settings
	QSettings settings;
	settings.setValue("core/size", size());
	settings.setValue("core/maximized", isMaximized());
	settings.setValue("core/fullscreen", isFullScreen());
	
	// Close the Database at the End
	db.close();
}

void PiTres::showStatusMessage(QString msg) {
	statusbar->showMessage(msg, 5000);
}

void PiTres::connectActions() {
	connect(actionFrom_LDAP, SIGNAL(triggered()), this, SLOT(importFromLDAP()));
	connect(action_About_PiTres, SIGNAL(triggered()), this, SLOT(showAbout()));
	connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
	connect(actionShow_Users, SIGNAL(triggered()), this, SLOT(showUsers()));
	connect(actionShow_Bills, SIGNAL(triggered()), this, SLOT(showSentBills()));
	connect(actionImport_Payments, SIGNAL(triggered()), this, SLOT(showImportPayments()));
	connect(actionConfiguration, SIGNAL(triggered()), this, SLOT(showSettings()));
	connect(actionInvoiceWizard, SIGNAL(triggered()), this, SLOT(showInvoiceWizard()));
	connect(actionContribution, SIGNAL(triggered()), this, SLOT(showContributions()));
	connect(actionBudget, SIGNAL(triggered()), this, SLOT(showBudget()));
}

void PiTres::debugAction(QString sender) {
	QString str(tr("The Action '%1' is not working in this release.").arg(sender));
	showStatusMessage(str);
}

void PiTres::setContent(QWidget* widget) {
	gridLayout->removeWidget(content);
	delete content;
	content = widget;
	gridLayout->addWidget(widget, 0, 0, 1, 1);
}

// SLOTS
void PiTres::importFromLDAP() {
	LDAPImport *widget = new LDAPImport;
	setContent(widget);
}

void PiTres::showAbout() {
	debugAction(QString("Show About"));
}

void PiTres::showHelp(int page) {
	debugAction(QString("Show Help (%1)").arg(page));
}

void PiTres::showUsers() {
	Userlist *widget = new Userlist;
	setContent(widget);
}

void PiTres::showSentBills() {
	SentBills *widget = new SentBills;
	setContent(widget);
}

void PiTres::showImportPayments() {
	PaymentImport *widget = new PaymentImport;
	setContent(widget);
}

void PiTres::showInvoiceWizard() {
	InvoiceWizard *widget = new InvoiceWizard;
	setContent(widget);
}

void PiTres::showContributions() {
	Contributions *widget = new Contributions;
	setContent(widget);
}

void PiTres::showBudget() {
	BudgetView *widget = new BudgetView;
	setContent(widget);
}

void PiTres::showSettings() {
	QSettings settings;
	
	settingsForm.sqliteFile->setText(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	settingsForm.memberAmountFull->setValue(settings.value("invoice/amount_limited", 30.0).toFloat());
	settingsForm.memberAmountLimited->setValue(settings.value("invoice/amount_default", 60.0).toFloat());
	settingsForm.memberDueDate->setText(settings.value("invoice/member_due_format", "yyyy-12-31").toString());
	
	settingsForm.pdfInvoice->setText(settings.value("pdf/invoice_template", "data/invoice.xml").toString());
	settingsForm.pdfReminder->setText(settings.value("pdf/reminder_template", "data/reminder.xml").toString());
	settingsForm.pdfReceipt->setText(settings.value("pdf/receipt_template", "data/receipt.xml").toString());
	
	settingsForm.assetType->setText(settings.value("qif/account_asset", "Oth A").toString());
	settingsForm.assetLabel->setText(settings.value("qif/payee_label", "Membership: ").toString());
	settingsForm.assetMemo->setText(settings.value("qif/memo", "Member UID: ").toString());
	settingsForm.assetAccountFull->setText(settings.value("qif/income_limited", "Membership Limited").toString());
	settingsForm.assetAccountLimited->setText(settings.value("qif/income_default", "Membership Default").toString());
	
	settingsForm.paymentType->setText(settings.value("qif/account_bank", "Bank").toString());
	settingsForm.paymentLabel->setText(settings.value("qif/payee_label", "Payment: ").toString());
	settingsForm.paymentMemo->setText(settings.value("qif/memo", "Member UID: ").toString());
	settingsForm.paymentAccount->setText(settings.value("qif/account_income", "Membership Fee").toString());
	
	settingsForm.ldapHost->setText(settings.value("ldap/server", "localhost").toString());
	settingsForm.ldapPort->setValue(settings.value("ldap/port", 389).toInt());
	settingsForm.ldapBindDn->setText(settings.value("ldap/binddn", "uniqueIdentifier=<Your UID here>,dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldapPassword->setText(settings.value("ldap/password", "").toString());
	settingsForm.ldapBaseDn->setText(settings.value("ldap/basedn", "dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldapSearch->setText(settings.value("ldap/search", "(objectClass=ppsPerson)").toString());
	settingsForm.ldapSectionRegex->setText(settings.value("ldap/sectionregex", "^.*,st=([a-z]{2}).*$|^.*,dc=(members).*$").toString());
	
	settingsForm.ldifNonsectionMembers->setText(settings.value("ldif/members_dn", "uniqueIdentifier=%1,dc=members,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldifSectionMembers->setText(settings.value("ldif/main_dn", "uniqueIdentifier=%1,dc=members,st=%2,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldifPaidDueAttribute->setText(settings.value("ldif/memberstate_attribute", "ppsVotingRightUntil").toString());
	settingsForm.ldifPaidDueReplace->setChecked(settings.value("ldif/replace_attribute", false).toBool());
	
	settingsForm.contributionPayer->setText(settings.value("contribution/payer", "Pirateparty Switzerland").toString());
	settingsForm.contributionMemo->setText(settings.value("contribution/memo", "Contribution: %1").toString());
	settingsForm.contributionAccount->setText(settings.value("contribution/account", "contribution_%1").toString());
	settingsForm.contributionIncome->setText(settings.value("contribution/income", "Membership fee").toString());
	settingsForm.contributionDontpay->setPlainText(settings.value("contribution/dontpay", "members").toStringList().join("\n"));
	
	settingsForm.donationPayer->setText(settings.value("donation/payer", "Pirateparty Switzerland").toString());
	settingsForm.donationMemo->setText(settings.value("donation/memo", "Donation: %1").toString());
	settingsForm.donationAccount->setText(settings.value("donation/account", "donation_%1").toString());
	settingsForm.donationIncome->setText(settings.value("donation/income", "Donation").toString());
	settingsForm.donationDontpay->setPlainText(settings.value("donation/dontpay", "members").toStringList().join("\n"));
	
	settingsForm.smtpHost->setText(settings.value("smtp/host", "localhost").toString());
	settingsForm.smtpPort->setValue(settings.value("smtp/port", "587").toInt());
	settingsForm.smtpUsername->setText(settings.value("smtp/username", "none@dom.tld").toString());
	settingsForm.smtpPassword->setText(settings.value("smtp/password", "").toString());
	settingsForm.smtpFrom->setText(settings.value("smtp/from", "none@dom.tld").toString());
	settingsForm.smtpAuthentication->setCurrentIndex(
		settings.value("smtp/authentication", "plain").toString() == "plain" ? 1 :
		settings.value("smtp/authentication", "plain").toString() == "login" ? 2 :
		0
	);
	
	settingsForm.pdfPPCounty->setText(settings.value("pdf/var_pp_country", "CH").toString());
	settingsForm.pdfPPZip->setText(settings.value("pdf/var_pp_zip", "1337").toString());
	settingsForm.pdfPPCity->setText(settings.value("pdf/var_pp_city", "Vallorbe").toString());
	settingsForm.pdfPrintCity->setText(settings.value("pdf/var_print_city", "Vallorbe").toString());
	settingsForm.pdfAccountNumber->setText(settings.value("pdf/bank_account_number", "01-84038-2").toString());
	settingsForm.pdfCurrency->setText(settings.value("pdf/esr_currency", "CHF").toString());
	settingsForm.pdfDateFormat->setText(settings.value("pdf/date_format", "dd.MM.yyyy").toString());
	
	settingsDialog->show();
}

void PiTres::doSaveSettings() {
	QSettings settings;
	
	settingsDialog->hide();
	
	settings.setValue("database/sqlite", settingsForm.sqliteFile->text());
	settings.setValue("invoice/amount_limited", settingsForm.memberAmountFull->value());
	settings.setValue("invoice/amount_default", settingsForm.memberAmountLimited->value());
	settings.setValue("invoice/member_due_format", settingsForm.memberDueDate->text());
	
	settings.setValue("pdf/invoice_template", settingsForm.pdfInvoice->text());
	settings.setValue("pdf/reminder_template", settingsForm.pdfReminder->text());
	settings.setValue("pdf/receipt_template", settingsForm.pdfReceipt->text());
	
	settings.setValue("qif/account_asset", settingsForm.assetType->text());
	settings.setValue("qif/payee_label", settingsForm.assetLabel->text());
	settings.setValue("qif/memo", settingsForm.assetMemo->text());
	settings.setValue("qif/income_limited", settingsForm.assetAccountFull->text());
	settings.setValue("qif/income_default", settingsForm.assetAccountLimited->text());
	
	settings.setValue("qif/account_bank", settingsForm.paymentType->text());
	settings.setValue("qif/payee_label", settingsForm.paymentLabel->text());
	settings.setValue("qif/memo", settingsForm.paymentMemo->text());
	settings.setValue("qif/account_income", settingsForm.paymentAccount->text());
	
	settings.setValue("ldap/server", settingsForm.ldapHost->text());
	settings.setValue("ldap/port", settingsForm.ldapPort->value());
	settings.setValue("ldap/binddn", settingsForm.ldapBindDn->text());
	settings.setValue("ldap/password", settingsForm.ldapPassword->text());
	settings.setValue("ldap/basedn", settingsForm.ldapBaseDn->text());
	settings.setValue("ldap/search", settingsForm.ldapSearch->text());
	settings.setValue("ldap/sectionregex", settingsForm.ldapSectionRegex->text());
	
	settings.setValue("ldif/members_dn", settingsForm.ldifNonsectionMembers->text());
	settings.setValue("ldif/main_dn", settingsForm.ldifSectionMembers->text());
	settings.setValue("ldif/memberstate_attribute", settingsForm.ldifPaidDueAttribute->text());
	settings.setValue("ldif/replace_attribute", settingsForm.ldifPaidDueReplace->isChecked());
	
	settings.setValue("contribution/payer", settingsForm.contributionPayer->text());
	settings.setValue("contribution/memo", settingsForm.contributionMemo->text());
	settings.setValue("contribution/account", settingsForm.contributionAccount->text());
	settings.setValue("contribution/income", settingsForm.contributionIncome->text());
	settings.setValue("contribution/dontpay", QStringList(settingsForm.contributionDontpay->toPlainText().split("\n")));
	
	settings.setValue("donation/payer", settingsForm.donationPayer->text());
	settings.setValue("donation/memo", settingsForm.donationMemo->text());
	settings.setValue("donation/account", settingsForm.donationAccount->text());
	settings.setValue("donation/income", settingsForm.donationIncome->text());
	settings.setValue("donation/dontpay", QStringList(settingsForm.donationDontpay->toPlainText().split("\n")));
	
	settings.setValue("smtp/host", settingsForm.smtpHost->text());
	settings.setValue("smtp/port", settingsForm.smtpPort->value());
	settings.setValue("smtp/username", settingsForm.smtpUsername->text());
	settings.setValue("smtp/password", settingsForm.smtpPassword->text());
	settings.setValue("smtp/from", settingsForm.smtpFrom->text());
	settings.setValue("smtp/authentication", settingsForm.smtpAuthentication->currentText());
	
	settings.setValue("pdf/var_pp_country", settingsForm.pdfPPCounty->text());
	settings.setValue("pdf/var_pp_zip", settingsForm.pdfPPZip->text());
	settings.setValue("pdf/var_pp_city", settingsForm.pdfPPCity->text());
	settings.setValue("pdf/var_print_city", settingsForm.pdfPrintCity->text());
	settings.setValue("pdf/bank_account_number", settingsForm.pdfAccountNumber->text());
	settings.setValue("pdf/esr_currency", settingsForm.pdfCurrency->text());
	settings.setValue("pdf/date_format", settingsForm.pdfDateFormat->text());
}

void PiTres::showInvoiceFileDialog() {
	QString file = QFileDialog::getOpenFileName(settingsDialog, tr("Select PDF-Template"), settingsForm.pdfInvoice->text(), tr("PDF-Templates (*.xml)"));
	if (!file.isEmpty()) {
		settingsForm.pdfInvoice->setText(file);
	}
}

void PiTres::showReminderFileDialog() {
	QString file = QFileDialog::getOpenFileName(settingsDialog, tr("Select PDF-Template"), settingsForm.pdfReminder->text(), tr("PDF-Templates (*.xml)"));
	if (!file.isEmpty()) {
		settingsForm.pdfReminder->setText(file);
	}
}

void PiTres::showReceiptFileDialog() {
	QString file = QFileDialog::getOpenFileName(settingsDialog, tr("Select PDF-Template"), settingsForm.pdfReceipt->text(), tr("PDF-Templates (*.xml)"));
	if (!file.isEmpty()) {
		settingsForm.pdfReceipt->setText(file);
	}
}
