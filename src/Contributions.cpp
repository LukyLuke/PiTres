
#include "Contributions.h"

#include "data/Invoice.h"
#include "data/Section.h"
#include "data/Person.h"
#include "helper/Smtp.h"

#include <cstdlib>
#include <cstdio>
#include <QFileDialog>
#include <QFile>
#include <QList>
#include <QSettings>

Contributions::Contributions(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(dateUntil, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	connect(btnEmail, SIGNAL(clicked()), this, SLOT(sendEmail()));
	
	tableModel = new QSqlQueryModel(tableView);
	loadData();
}

Contributions::~Contributions() {}

void Contributions::loadData() {
	Invoice::createTables();
	
	dateFrom->setDate(QDate::currentDate().addMonths(-4));
	dateUntil->setDate(QDate::currentDate());
	
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("Paid Date"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Person"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("City"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("Section"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("Member UID"));
	tableModel->setHeaderData(6, Qt::Horizontal, tr("Reference-Number"));
	
	tableView->setSortingEnabled(false);
	tableView->setModel(tableModel);
}

QSqlQuery Contributions::createQuery() {
	QSqlQuery query(db);
	query.prepare("SELECT paid_date,amount,address_name,address_city,for_section,member_uid,reference FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end ORDER BY for_section ASC;");
	query.bindValue(0, dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(1, dateUntil->date().toString("yyyy-MM-dd"));
	return query;
}

void Contributions::searchData() {
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	showOverview();
}

void Contributions::showOverview() {
	QSettings settings;
	QSqlQuery query(db);
	
	// Clean the GridContainer
	QLayoutItem *child;
	while ((child = overviewLayout->takeAt(0)) != 0) {
		delete ((QLabel *)child->widget());
		delete child;
	}
	
	// Load "do not contribute this sections" list
	QList<QString> dontContribute = settings.value("contributions/dontpay", "members").toStringList();
	
	// Refill with Data
	query.prepare("SELECT SUM(amount/2) AS amount,for_section FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end GROUP BY for_section;");
	query.bindValue(0, dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(1, dateUntil->date().toString("yyyy-MM-dd"));
	query.exec();
	double sum = 0.0;
	int row = 0;
	while (query.next()) {
		if (!dontContribute.contains(query.value(1).toString())) {
			QLabel *s = new QLabel( tr("Section: %1").arg(query.value(1).toString()) );
			QLabel *a = new QLabel( tr("%1 sFr.").arg(query.value(0).toDouble(), 0, 'f', 2) );
			a->setAlignment(Qt::AlignRight);
			overviewLayout->addWidget(s, row, 0);
			overviewLayout->addWidget(a, row, 1);
			
			sum += query.value(0).toDouble();
			row++;
		}
	}
	
	// Total
	QLabel *s = new QLabel( tr("<b>Total:</b>") );
	QLabel *a = new QLabel( tr("<b>%1 sFr.</b>").arg(sum, 0, 'f', 2) );
	a->setAlignment(Qt::AlignRight);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(a, row, 1);
}

void Contributions::createQif() {
	QSettings settings;
	QString valuta = QDate::currentDate().toString("yyyy-MM-dd");
	QString date = QDate::currentDate().toString("yyyyMMdd");
	QString memo = settings.value("qif/payee_label", "Contribution: ").toString();
	QString section, amount;
	QSqlQuery query2(db);
	QSqlQuery query(db);
	
	// Load "do not contribute this sections" list
	QList<QString> dontContribute = settings.value("contributions/dontpay", "members").toStringList();
	
	// Initialize the QIF and the list with all Section-QIFs
	sectionQif.clear();
	QString qif_national("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	if (!memoText->text().isEmpty()) {
		memo = memoText->text();
	}
	
	// Get all Data
	query.prepare("SELECT SUM(amount/2) AS amount,for_section FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end GROUP BY for_section;");
	query.bindValue(0, dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(1, dateUntil->date().toString("yyyy-MM-dd"));
	query.exec();
	while (query.next()) {
		amount = query.value(0).toString();
		section = query.value(1).toString();
		
		// Don't generate a QIF if the section is not getting paid...
		if (dontContribute.contains(section)) {
			continue;
		}
		
		// National Liability QIF
		qif_national.append("\nD" + valuta);
		qif_national.append("\nT" + amount);
		qif_national.append("\nP"+ settings.value("qif/contribution_payer", "Pirateparty Switzerland").toString().arg(section));
		qif_national.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_national.append("\nM" + memo.arg(section));
		qif_national.append("\nL" + settings.value("qif/account_contribution", "Contribution_%1").toString().arg(section));
		qif_national.append("\n^\n");
		
		// Section Income QIF
		QString qif_section("!Type:" + settings.value("qif/account_bank", "Bank").toString());
		qif_section.append("\nD" + valuta);
		qif_section.append("\nT" + amount);
		qif_section.append("\nP"+ settings.value("qif/contribution_payer", "Pirateparty Switzerland").toString().arg(section));
		qif_section.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_section.append("\nM" + memo.arg(section));
		qif_section.append("\nL" + settings.value("qif/account_income", "Membership Fee").toString());
		qif_section.append("\n^\n");
		
		sectionQif.insert(section, qif_section);
	}
	
	// Safe the Liabilities-QIF
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Liabilities for the payer"), "", tr("Quicken Interchange Format (*.qif)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif_national;
			out.flush();
			f.close();
		}
	}
}

void Contributions::exportData() {
	createQif();
	
	// Safe the Contributions-QIF
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Contributions for the receiver"), "", tr("Quicken Interchange Format (*.qif)"));
	if (!fileName.isEmpty()) {
		QList<QString>::const_iterator section;
		QList<QString> keys = sectionQif.keys();
		for (section = keys.constBegin(); section != keys.constEnd(); section++) {
			QFile f(QString(fileName).replace(".qif", "_" + (*section) + ".qif"));
			if (f.open(QFile::WriteOnly | QFile::Truncate)) {
				QTextStream out(&f);
				out << sectionQif.value(*section);
				out.flush();
				f.close();
			}
		}
	}
}

void Contributions::sendEmail() {
	QSettings settings;
	Section sec;
	PPSPerson pers;
	QString subject, message, email;
	
	createQif();
	
	// TODO: Check if this is working with current implementation or do we have to clean the attachments and other settings before we send the next mail?
	Smtp mail(settings.value("smtp/host", "localhost").toString(), settings.value("smtp/port", 587).toInt());
	if (settings.value("smtp/authentication", "login").toString() == "login") {
		mail.authLogin(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	} else {
		mail.authPlain(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	}
	
	// Send the Contributions by EMail
	QString attachment = "Contribution_" + QDate::currentDate().toString("yyyy-MM-dd") + ".qif";
	QList<QString>::const_iterator section;
	QList<QString> keys = sectionQif.keys();
	for (section = keys.constBegin(); section != keys.constEnd(); section++) {
		// Create a tmp file form the QIF
		char fname [L_tmpnam];
		tmpnam(fname);
		QString fileName(fname);
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << sectionQif.value(*section);
			out.flush();
			f.close();
		}
		mail.clearAttachments();
		mail.attach(fileName, attachment);
		
		// Load the treasurer of the current Section
		sec.load(*section);
		pers.load(sec.treasurer());
		
		// TODO: Load Subject and Texts
		subject = QString("Contributions for %1").arg(*section);
		message = "Attatched the Contributions:\n...blubberl blah\n\nGreetings your Treasurer";
		
		// Set the Mailtext and send the mail
		mail.setTextMessage(message);
		email = "";
		if (pers.email().length() > 0) {
			email = pers.email().at(0);
		}
		qDebug() << "Sending contributions to: " << email;
		// TODO: Remove if working
		email = "postmaster@ranta.ch";
		
		// If sending was not successfull (or no treasurer was found), let the user save the QIF
		if (!mail.send(settings.value("smtp/from", "me@noreply.dom").toString(), email, subject)) {
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save Contribution for the section"), "", tr("Quicken Interchange Format (*.qif)"));
			if (!fileName.isEmpty()) {
				if (QFile::exists(fileName)) {
					QFile::remove(fileName);
				}
				if (!QFile::copy(fname, fileName)) {
					qDebug() << "Error while copying the QIF: " << fname << " to " << fileName;
				}
			}
		}
		
		// remove the tmp file
		remove(fname);
	}
}
