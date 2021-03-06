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

PiTres::PiTres(QMainWindow *parent) : QMainWindow(parent) {
#ifdef SMTP_DEBUG
	qDebug() << "Compiled with SMTP_DEBUG: No emails are sent out.";
#endif
#ifdef SMTP_SAVE_DEBUG
	qDebug() << "Compiled with SMTP_SAVE_DEBUG: Emails are stored in the debug folder.";
#endif
#ifdef FIO
	qDebug() << "Compiled with FIO: The Financial-Regulation Membership-Fee calculation is used with individual recommendations for each individual section.";
#else
	qDebug() << "Compiled without FIO: The standard Membershipfee calculation is used with two Membership fee amounts.";
#endif
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
	_settingsShowed = FALSE;
	settingsForm.setupUi(settingsDialog);
	connect(settingsForm.actionSave, SIGNAL(triggered()), this, SLOT(doSaveSettings()));
	
	connect(settingsForm.invoiceSelect, SIGNAL(clicked()), this, SLOT(showInvoiceFileDialog()));
	connect(settingsForm.reminderSelect, SIGNAL(clicked()), this, SLOT(showReminderFileDialog()));
	connect(settingsForm.receiptSelect, SIGNAL(clicked()), this, SLOT(showReceiptFileDialog()));
	connect(settingsForm.statisticSelect, SIGNAL(clicked()), this, SLOT(showStatisticFileDialog()));
	connect(settingsForm.contributionSelect, SIGNAL(clicked()), this, SLOT(showContributionFileDialog()));
	
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
	Donation::createTables();
	
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
	connect(actionEditSections, SIGNAL(triggered()), this, SLOT(showSectionEdit()));
	connect(actionDonations, SIGNAL(triggered()), this, SLOT(showDonations()));
	connect(actionStatistics, SIGNAL(triggered()), this, SLOT(showStatistics()));
	connect(actionDonationWizard, SIGNAL(triggered()), this, SLOT(showDonationWizard()));
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

void PiTres::showSectionEdit() {
	SectionEdit *widget = new SectionEdit;
	setContent(widget);
}

void PiTres::showDonations() {
	Donations *widget = new Donations;
	setContent(widget);
}

void PiTres::showDonationWizard() {
	DonationWizard *wizard = new DonationWizard(this);
	wizard->show();
}

void PiTres::showStatistics() {
	Statistics *widget = new Statistics;
	setContent(widget);
}

void PiTres::showSettings() {
	QSettings settings;
	QDate now = QDate::currentDate();
	
	settingsForm.sqliteFile->setText(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	settingsForm.memberDueDate->setText(settings.value("invoice/member_due_format", "yyyy-02-15").toString());
	settingsForm.memberDueDateNextYear->setChecked(settings.value("invoice/member_due_next_year", TRUE).toBool());
#ifndef FIO
	settingsForm.memberAmountFull->setValue(settings.value("invoice/amount_default", 30.0).toFloat());
	settingsForm.memberAmountLimited->setValue(settings.value("invoice/amount_limited", 60.0).toFloat());
	if (!_settingsShowed) {
		delete settingsForm.labelrecommendationsMaxSections;
		delete settingsForm.maxNumSections;
		delete settingsForm.memberAmountMin;
		delete settingsForm.labelMemberAmountMinimum;
	}
#else
	settingsForm.memberAmountFull->setValue(settings.value("invoice/recommendation_maximum", 125.0).toFloat());
	settingsForm.memberAmountLimited->setValue(settings.value("invoice/recommendation_none", 15.0).toFloat());
	settingsForm.maxNumSections->setValue(settings.value("invoice/recommendation_numsections", 4).toInt());
	settingsForm.labelMemberAmountFull->setText(tr("Maximum recommendation per Section"));
	settingsForm.labelMemberAmountLimited->setText(tr("Default recommendation"));
	settingsForm.memberAmountMin->setValue(settings.value("invoice/minimum_amount", 30.0).toFloat());
#endif
	
	settingsForm.pdfInvoice->setText(settings.value("pdf/invoice_template", "data/invoice.xml").toString());
	settingsForm.pdfReminder->setText(settings.value("pdf/reminder_template", "data/reminder.xml").toString());
	settingsForm.pdfReceipt->setText(settings.value("pdf/receipt_template", "data/receipt.xml").toString());
	settingsForm.pdfStatistic->setText(settings.value("pdf/statistic_template", "data/statistic.xml").toString());
	settingsForm.pdfContribution->setText(settings.value("pdf/contribution_template", "data/contribution.xml").toString());
	
	settingsForm.assetType->setText(settings.value("qif/account_asset", "Oth A").toString());
	settingsForm.assetLabel->setText(settings.value("qif/invoice_label", "Membership: %1 (%2)").toString());
	settingsForm.assetMemo->setText(settings.value("qif/memo", "Member UID: ").toString());
	settingsForm.assetAccountFull->setText(settings.value("qif/income_default", "Membership Default %1").toString());
#ifdef FIO
	if (!_settingsShowed) {
		delete settingsForm.assetAccountLimited;
		delete settingsForm.labelAssetsAccountLimited;
	}
	settingsForm.labelAssetsAccount->setText(tr("Income Account"));
#else
	settingsForm.assetAccountLimited->setText(settings.value("qif/income_limited", "Membership Limited %2").toString());
#endif
	
	settingsForm.paymentType->setText(settings.value("qif/account_bank", "Bank").toString());
	settingsForm.paymentLabel->setText(settings.value("qif/payee_label", "Payment:  %1 (%2)").toString());
	settingsForm.paymentMemo->setText(settings.value("qif/memo", "Member UID: %1").toString());
	settingsForm.paymentAccount->setText(settings.value("qif/account_income", "Membership fee %1").toString());
	
	settingsForm.ldapHost->setText(settings.value("ldap/server", "localhost").toString());
	settingsForm.ldapPort->setValue(settings.value("ldap/port", 389).toInt());
	settingsForm.ldapBindDn->setText(settings.value("ldap/binddn", "uniqueIdentifier=<Your UID here>,dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldapPassword->setText(settings.value("ldap/password", "").toString());
	settingsForm.ldapBaseDn->setText(settings.value("ldap/basedn", "dc=members,st=<Your Section here>,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldapSearch->setText(settings.value("ldap/search", "(objectClass=ppsPerson)").toString());
	settingsForm.ldapSectionRegex->setText(settings.value("ldap/sectionregex", "^.*,l=([a-z]+),(st|l)=.*$||^.*,st=([a-z]{2}).*$||^.*,dc=(members).*$").toString());
	
	settingsForm.ldifNonsectionMembers->setText(settings.value("ldif/members_dn", "uniqueIdentifier=%1,dc=members,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldifSectionMembers->setText(settings.value("ldif/main_dn", "uniqueIdentifier=%1,dc=members,%3st=%2,dc=piratenpartei,dc=ch").toString());
	settingsForm.ldifSubSection->setText(settings.value("ldif/subsection_concat", "l=%1,").toString());
	settingsForm.ldifPaidDueAttribute->setText(settings.value("ldif/membertype_attribute", "employeeType").toString());
	settingsForm.ldifPaidDueAttribute->setText(settings.value("ldif/memberstate_attribute", "ppsVotingRightUntil").toString());
	settingsForm.ldifPaidDueReplace->setChecked(settings.value("ldif/replace_attribute", false).toBool());
	
	settingsForm.contributionPayer->setText(settings.value("contribution/payer", "Pirateparty Switzerland").toString());
	settingsForm.contributionMemo->setText(settings.value("contribution/memo", "Contribution: %1").toString());
	settingsForm.contributionAccount->setText(settings.value("contribution/account", "Contribution %1").toString());
	settingsForm.contributionIncome->setText(settings.value("contribution/income", "Membership fee %1").toString());
	settingsForm.contributionDontpay->setPlainText(settings.value("contribution/dontpay", "members").toStringList().join("\n"));
	
	settingsForm.donationPayer->setText(settings.value("donation/payer", "Pirateparty Switzerland").toString());
	settingsForm.donationMemo->setText(settings.value("donation/memo", "Donation: %1").toString());
	settingsForm.donationAccount->setText(settings.value("donation/account", "Donation %1").toString());
	settingsForm.donationIncome->setText(settings.value("donation/income", "Donation %1").toString());
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
	settingsForm.smtpEncryption->setCurrentIndex(
		settings.value("smtp/use_tls", FALSE).toBool() ? 1 :
		settings.value("smtp/use_ssl", FALSE).toBool() ? 2 :
		0
	);
	
	settingsForm.pdfPPCounty->setText(settings.value("pdf/var_pp_country", "CH").toString());
	settingsForm.pdfPPZip->setText(settings.value("pdf/var_pp_zip", "1337").toString());
	settingsForm.pdfPPCity->setText(settings.value("pdf/var_pp_city", "Vallorbe").toString());
	settingsForm.pdfPrintCity->setText(settings.value("pdf/var_print_city", "Vallorbe").toString());
	settingsForm.pdfAccountNumber->setText(settings.value("pdf/bank_account_number", "01-84038-2").toString());
	settingsForm.pdfCurrency->setText(settings.value("pdf/esr_currency", "CHF").toString());
	settingsForm.pdfDateFormat->setText(settings.value("pdf/date_format", "dd.MM.yyyy").toString());
	settingsForm.pdfEmailPrepend->setText(settings.value("pdf/email_prepend", "info").toString());
	settingsForm.pdfEmailAppend->setText(settings.value("pdf/email_append", "piratenpartei.ch").toString());
	
	settingsForm.dateContributionQ1->setDate(settings.value("payment/contribution_q1", QDate(now.year(), 3, 31)).toDate());
	settingsForm.dateContributionQ2->setDate(settings.value("payment/contribution_q2", QDate(now.year(), 6, 30)).toDate());
	settingsForm.dateContributionQ3->setDate(settings.value("payment/contribution_q3", QDate(now.year(), 9, 30)).toDate());
	settingsForm.dateContributionQ4->setDate(settings.value("payment/contribution_q4", QDate(now.year(), 12, 31)).toDate());
	
	_settingsShowed = TRUE;
	settingsDialog->show();
}

void PiTres::doSaveSettings() {
	QSettings settings;
	
	settingsDialog->hide();
	
	settings.setValue("database/sqlite", settingsForm.sqliteFile->text());
	settings.setValue("invoice/member_due_format", settingsForm.memberDueDate->text());
	settings.setValue("invoice/member_due_next_year", settingsForm.memberDueDateNextYear->isChecked());
#ifndef FIO
	settings.setValue("invoice/amount_default", settingsForm.memberAmountFull->value());
	settings.setValue("invoice/amount_limited", settingsForm.memberAmountLimited->value());
#else
	settings.setValue("invoice/recommendation_maximum", settingsForm.memberAmountFull->value());
	settings.setValue("invoice/recommendation_none", settingsForm.memberAmountLimited->value());
	settings.setValue("invoice/recommendation_numsections", settingsForm.maxNumSections->value());
	settings.setValue("invoice/minimum_amount", settingsForm.memberAmountMin->value());
#endif
	
	settings.setValue("pdf/invoice_template", settingsForm.pdfInvoice->text());
	settings.setValue("pdf/reminder_template", settingsForm.pdfReminder->text());
	settings.setValue("pdf/receipt_template", settingsForm.pdfReceipt->text());
	settings.setValue("pdf/statistic_template", settingsForm.pdfStatistic->text());
	settings.setValue("pdf/contribution_template", settingsForm.pdfContribution->text());
	
	settings.setValue("qif/account_asset", settingsForm.assetType->text());
	settings.setValue("qif/invoice_label", settingsForm.assetLabel->text());
	settings.setValue("qif/memo", settingsForm.assetMemo->text());
	settings.setValue("qif/income_default", settingsForm.assetAccountFull->text());
#ifndef FIO
	settings.setValue("qif/income_limited", settingsForm.assetAccountLimited->text());
#endif
	
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
	settings.setValue("ldif/subsection_concat", settingsForm.ldifSubSection->text());
	settings.setValue("ldif/memberstate_attribute", settingsForm.ldifPaidDueAttribute->text());
	settings.setValue("ldif/membertype_attribute", settingsForm.ldifMembertypeAttribute->text());
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
	settings.setValue("smtp/use_tls", settingsForm.smtpEncryption->currentIndex() == 1);
	settings.setValue("smtp/use_ssl", settingsForm.smtpEncryption->currentIndex() == 2);
	
	settings.setValue("pdf/var_pp_country", settingsForm.pdfPPCounty->text());
	settings.setValue("pdf/var_pp_zip", settingsForm.pdfPPZip->text());
	settings.setValue("pdf/var_pp_city", settingsForm.pdfPPCity->text());
	settings.setValue("pdf/var_print_city", settingsForm.pdfPrintCity->text());
	settings.setValue("pdf/bank_account_number", settingsForm.pdfAccountNumber->text());
	settings.setValue("pdf/esr_currency", settingsForm.pdfCurrency->text());
	settings.setValue("pdf/date_format", settingsForm.pdfDateFormat->text());
	settings.setValue("pdf/email_prepend", settingsForm.pdfEmailPrepend->text());
	settings.setValue("pdf/email_append", settingsForm.pdfEmailPrepend->text());
	
	settings.setValue("payment/contribution_q1", settingsForm.dateContributionQ1->date());
	settings.setValue("payment/contribution_q2", settingsForm.dateContributionQ2->date());
	settings.setValue("payment/contribution_q3", settingsForm.dateContributionQ3->date());
	settings.setValue("payment/contribution_q4", settingsForm.dateContributionQ4->date());
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

void PiTres::showStatisticFileDialog() {
	QString file = QFileDialog::getOpenFileName(settingsDialog, tr("Select PDF-Template"), settingsForm.pdfReceipt->text(), tr("PDF-Templates (*.xml)"));
	if (!file.isEmpty()) {
		settingsForm.pdfStatistic->setText(file);
	}
}

void PiTres::showContributionFileDialog() {
	QString file = QFileDialog::getOpenFileName(settingsDialog, tr("Select PDF-Template"), settingsForm.pdfReceipt->text(), tr("PDF-Templates (*.xml)"));
	if (!file.isEmpty()) {
		settingsForm.pdfContribution->setText(file);
	}
}
