
#include "PaymentImport.h"

#include <QVariant>
#include <QDate>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QRegExp>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>

PaymentImport::PaymentImport(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	dateEdit->setDate(QDate::currentDate());
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(btnContinue, SIGNAL(clicked()), this, SLOT(continueImport()));
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(paySelectedInvoice()));
	connect(btnInvoiceQif, SIGNAL(clicked()), this, SLOT(exportQifLiabilities()));
	
	openDatabase();
}
PaymentImport::~PaymentImport() {
	//db.close();
}

void PaymentImport::openDatabase() {
	QSettings settings;
	QFileInfo dbfile(settings.value("database/sqlite", "data/userlist.sqlite").toString());
	qDebug() << "Loading DB: " + dbfile.absoluteFilePath();
	
	if (!dbfile.absoluteDir().exists()) {
		dbfile.absoluteDir().mkpath(dbfile.absolutePath());
		qDebug() << "Path did not exists, created: " + dbfile.absolutePath();
	}
	
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbfile.absoluteFilePath());
	db.open();
}

void PaymentImport::timerEvent(QTimerEvent *event) {
	killTimer(searchTimer);
	searchData();
}

void PaymentImport::searchData() {
	listAvailable->clear();
	if (searchEdit->text().isEmpty()) {
		return;
	}
	
	QString search;
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	for (int i = 0; i < sl.size(); i++) {
		if (i > 0) {
			search.append(" AND ");
		}
		search.append(QString("(reference LIKE :p1_").append(QString::number(i)).append(" OR member_uid = :p2_").append(QString::number(i)).append(" OR address_name LIKE :p3_").append(QString::number(i)).append(")"));
	}
	
	QSqlQuery query(db);
	query.prepare(QString("SELECT member_uid,reference,payable_date,address_name,for_section FROM pps_invoice WHERE %1;").arg(search));
	
	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":p1_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p2_").append(QString::number(i)), sl.at(i));
		query.bindValue(QString(":p3_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
	}
	
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
		return;
	}
	
	int fieldMember = query.record().indexOf("member_uid");
	int fieldReference = query.record().indexOf("reference");
	int fieldName = query.record().indexOf("address_name");
	int fieldSection = query.record().indexOf("for_section");
	int fieldPayable = query.record().indexOf("payable_date");
	while (query.next()) {
		listAvailable->addItem(QString("%1 (%2): %3 (%4) / Section %5").arg(
			query.value(fieldReference).toString(),
			query.value(fieldPayable).toDate().toString("yyyy-MM-dd"),
			query.value(fieldName).toString(),
			query.value(fieldMember).toString(),
			query.value(fieldSection).toString()
		));
	}
}

void PaymentImport::searchDataTimeout(QString data) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void PaymentImport::paySelectedInvoice() {
	if ((listAvailable->currentRow() > -1) && !editAmount->text().isEmpty()) {
		QString text = listAvailable->currentItem()->text();
		QRegExp re("^([\\d ]+) \\(([\\d-]+)\\): ([^(]+)\\((\\d+)\\) / Section (.*)$");
		if (re.indexIn(text) > -1) {
			int row = tablePay->rowCount();
			tablePay->insertRow(row);
			tablePay->setItem(row, 0, new QTableWidgetItem( re.cap(1) ));
			tablePay->setItem(row, 1, new QTableWidgetItem(editAmount->text()));
			tablePay->setItem(row, 2, new QTableWidgetItem( re.cap(4) ));
			tablePay->setItem(row, 3, new QTableWidgetItem(dateEdit->date().toString("yyyy-MM-dd")));
			tablePay->setItem(row, 4, new QTableWidgetItem( re.cap(3) ));
			tablePay->setItem(row, 5, new QTableWidgetItem( re.cap(2) ));
			tablePay->setItem(row, 6, new QTableWidgetItem( re.cap(5) ));
		} else {
			qDebug() << "Could not parse the TextLine, sorry...";
		}
	}
}

void PaymentImport::continueImport() {
	exportQifBankTransfer();
	exportLDIFChanges();
}

void PaymentImport::exportLDIFChanges() {
	QSettings settings;
	QString ldiff;
	
	for (int i = 0; i < tablePay->rowCount(); i++) {
		QString member = tablePay->item(i, 2)->text();
		QString section = tablePay->item(i, 6)->text();
		
		if (section.isEmpty() || section.length() > 2) {
			ldiff.append("dn: " + settings.value("ldif/members_dn", "uniqueIdentifier=%1,dc=members,dc=piratenpartei,dc=ch").toString().arg(member));
		} else {
			ldiff.append("dn: " + settings.value("ldif/main_dn", "uniqueIdentifier=%1,dc=members,st=%2,dc=piratenpartei,dc=ch").toString().arg(member, section));
		}
		ldiff.append("\nchangetype: modify\ndelete: ");
		ldiff.append(settings.value("ldif/memberstate_attribute", "ppsPaid").toString());
		ldiff.append("\n-\nadd: ");
		ldiff.append(settings.value("ldif/memberstate_attribute", "ppsPaid").toString());
		ldiff.append("\n");
		ldiff.append(settings.value("ldif/memberstate_attribute", "ppsPaid").toString() + ": " + settings.value("ldif/memberstate_value", "1").toString());
		ldiff.append("\n\n");
	}
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save LDIF File"), "", tr("LDIF (*.ldif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << ldiff;
		}
	}
}

void PaymentImport::exportQifBankTransfer() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_asset", "Bank").toString());
	
	for (int i = 0; i < tablePay->rowCount(); i++) {
		QString ref = tablePay->item(i, 0)->text();
		QString amount = tablePay->item(i, 1)->text();
		QString member = tablePay->item(i, 2)->text();
		QString valuta = tablePay->item(i, 3)->text();
		QString name = tablePay->item(i, 4)->text();
		
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP"+ settings.value("qif/payee_label", tr("Payment: ")).toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", tr("Member UID: ")).toString() + member);
		qif.append("\nL" + settings.value("qif/account_income", "Membership Fee").toString());
		qif.append("\n^\n");
	}
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
}

void PaymentImport::exportQifLiabilities() {
	QSettings settings;
	QString qif("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	
	for (int i = 0; i < tablePay->rowCount(); i++) {
		QString ref = tablePay->item(i, 0)->text();
		QString amount = tablePay->item(i, 1)->text();
		QString member = tablePay->item(i, 2)->text();
		QString valuta = tablePay->item(i, 3)->text();
		QString name = tablePay->item(i, 4)->text();
		
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP" + settings.value("qif/payee_label", tr("Invoice: ")).toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", tr("Member UID: ")).toString() + member);
		qif.append("\nL" + settings.value("qif/account_income", "Membership Income").toString());
		qif.append("\n^\n");
	}
	
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
}
