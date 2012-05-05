
#include "PiTres.h"
#include "Userlist.h"
#include "SentBills.h"
#include "PaymentImport.h"
#include "LDAPImport.h"

#include <QtGui>
#include <QtGui/QGridLayout>
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
}

PiTres::~PiTres() {
	delete content;
	
	// Store current Window-Settings
	QSettings settings;
	settings.setValue("core/size", size());
	settings.setValue("core/maximized", isMaximized());
	settings.setValue("core/fullscreen", isFullScreen());
}

void PiTres::showStatusMessage(QString msg) {
	statusbar->showMessage(msg, 5000);
}

void PiTres::connectActions() {
	connect(action_Open, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(action_Recently_opend, SIGNAL(triggered()), this, SLOT(openRecently()));
	connect(actionFrom_LDAP, SIGNAL(triggered()), this, SLOT(importFromLDAP()));
	connect(actionFrom_File, SIGNAL(triggered()), this, SLOT(importFromFile()));
	connect(actionFrom_Clipboard, SIGNAL(triggered()), this, SLOT(importFromClipboard()));
	connect(action_About_PiTres, SIGNAL(triggered()), this, SLOT(showAbout()));
	connect(actionHelp, SIGNAL(triggered()), this, SLOT(showHelp()));
	connect(actionShow_Users, SIGNAL(triggered()), this, SLOT(showUsers()));
	connect(actionShow_Bills, SIGNAL(triggered()), this, SLOT(showSentBills()));
	connect(actionImport_Payments, SIGNAL(triggered()), this, SLOT(showImportPayments()));
	connect(actionConfiguration, SIGNAL(triggered()), this, SLOT(showSettings()));
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
void PiTres::openFile() {
	debugAction(QString("Open File"));
}

void PiTres::openRecently() {
	debugAction(QString("Open Recently"));
}

void PiTres::importFromLDAP() {
	LDAPImport *widget = new LDAPImport;
	setContent(widget);
}

void PiTres::importFromFile() {
	debugAction(QString("Import from File"));
}

void PiTres::importFromClipboard() {
	debugAction(QString("Import from Clipboard"));
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
#ifdef USE_SQLITE
//	SqliteStorage storage;
//	storage.load("data/userlist.db");
#endif
}

void PiTres::showSentBills() {
	SentBills *widget = new SentBills;
	setContent(widget);
}

void PiTres::showImportPayments() {
	PaymentImport *widget = new PaymentImport;
	setContent(widget);
}

void PiTres::showSettings() {
	settingsDialog->show();
}

void PiTres::doSaveSettings() {
	settingsDialog->hide();
}