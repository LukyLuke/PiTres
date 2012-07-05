
#include "Contributions.h"
#include "Invoice.h"
#include "../helper/Smtp.h"

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
	query.bindParam(0, dateFrom->date().toString("yyyy-MM-dd"));
	query.bindParam(1, dateUntil->date().toString("yyyy-MM-dd"));
	return query;
}

void Contributions::searchData() {
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
}

void Contributions::createQif() {
	QString valuta = QDate::currentDate.toString("yyyy-MM-dd");
	QString section;
	QSqlQuery query2(db);
	QSqlQuery query(db);
	query.prepare("SELECT for_section FROM pps_invoice GROUP BY for_section;");
	query.exec();
	
	sectionQif.clear();
	QString qif_national("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	
	while (query.next()) {
		section = query.value(0).toString();
		query2.prepare("SELECT SUM(amount/2) AS amount FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end AND for_section=?;");
		query2.bindParam(0, dateFrom->date().toString("yyyy-MM-dd"));
		query2.bindParam(1, dateUntil->date().toString("yyyy-MM-dd"));
		query2.bindParam(2, section);
		query2.exec();
		
		// National Liability QIF
		qif_national.append("\nD" + valuta);
		qif_national.append("\nT" + query2.value(0).toString());
		qif_national.append("\nP"+ settings.value("qif/payee_label", "Contribution: ").toString() + section);
		qif_national.append("\nN");
		qif_national.append("\nM" + tr("Contribution for ") + section);
		qif_national.append("\nL" + settings.value("qif/account_contribution", "Section ").toString() + section);
		qif_national.append("\n^\n");
		
		// Section Income QIF
		QString qif_section("!Type:" + settings.value("qif/account_bank", "Bank").toString());
		qif_section.append("\nD" + valuta);
		qif_section.append("\nT" + query2.value(0).toString());
		qif_section.append("\nP"+ settings.value("qif/payee_label", "Contribution: ").toString() + section);
		qif_section.append("\nN");
		qif_section.append("\nM" + tr("Contribution for ") + section);
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
			QFile f(fileName.replace(".qif", "_" + (*section) + ".qif"));
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
		mail.attach(fileName, attachment);
		
		// TODO: Load the treasurer of the current Section
		
		// Set the Mailtext and send the mail
		mail.setTextMessage("Attatched the Contributions:\n...");
		mail.send(settings.value("smtp/from", "me@noreply.dom").toString(), "postmaster@ranta.ch", "Contributions...");
		
		// TODO: If sending was not successfull (or no treasurer was found), let the user save the QIF
		
		// remove the tmp file
		remove(fname);
	}
}
