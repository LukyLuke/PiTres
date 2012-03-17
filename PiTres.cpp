#include <iostream>

#include <QtGui>
#include <QtGui/QGridLayout>

#include "PiTres.h"

PiTres::PiTres(QMainWindow *parent) {
	setupUi(this);
	content = new QWidget();
	connectActions();
}

PiTres::~PiTres() {
	delete content;
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
	connect(actionCreate_Bills, SIGNAL(triggered()), this, SLOT(showCreateBills()));
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
	debugAction(QString("Import from LDAP"));
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
	Userlist *users = new Userlist;
	setContent(users);
}

void PiTres::showSentBills() {
	SentBills *bills = new SentBills;
	setContent(bills);
}

void PiTres::showCreateBills() {
	debugAction(QString("show create bills"));
}
