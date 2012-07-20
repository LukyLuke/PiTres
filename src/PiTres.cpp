
#include "PiTres.h"
#include "Userlist.h"
#include "SentBills.h"
#include "PaymentImport.h"
#include "LDAPImport.h"
#include "InvoiceWizard.h"
#include "Contributions.h"

#include "data/Person.h"
#include "helper/XmlPdf.h"

#include <QtGui>
#include <QFileDialog>
#include <QtGui/QGridLayout>
#include <QFileInfo>
#include <QSettings>

PiTres::PiTres(QMainWindow *parent) : QMainWindow(parent) {
	setupUi(this);
	
	// Resize
	QSettings settings;
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
