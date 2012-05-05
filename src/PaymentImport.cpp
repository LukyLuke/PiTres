
#include "PaymentImport.h"
#include "data/Invoice.h"

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
	
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	QString search("reference LIKE :reference OR (");
	for (int i = 0; i < sl.size(); i++) {
		if (i > 0) {
			search.append(" AND ");
		}
		search.append("(");
		search.append( QString("member_uid = :uid_").append(QString::number(i)) );
		search.append( QString(" OR address_name LIKE :name_").append(QString::number(i)) );
		search.append(")");
	}
	search.append(")");
	
	QSqlQuery query(db);
	query.prepare(QString("SELECT member_uid,reference,payable_date,address_name,for_section FROM pps_invoice WHERE %1;").arg(search));
	
	query.bindValue(QString(":reference"), QString("%%1%").arg(sl.join(" ")));
	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":uid_").append(QString::number(i)), sl.at(i));
		query.bindValue(QString(":name_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
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
	QSettings settings;
	QString fileName;
	//float amountLimited = settings.value("invoice/amount_limited", 30.0).toFloat();
	//float amountDefault = settings.value("invoice/amount_default", 60.0).toFloat();
	QString qif("!Type:" + settings.value("qif/account_bank", "Bank").toString());
	QSqlQuery query(db);
	query.prepare("UPDATE pps_invoice SET paid_date=:date,amount_paid=amount_paid+:amount WHERE reference=:ref AND member_uid=:member;");
	
	for (int i = 0; i < tablePay->rowCount(); i++) {
		QString ref = tablePay->item(i, 0)->text();
		QString amount = tablePay->item(i, 1)->text();
		QString member = tablePay->item(i, 2)->text();
		QString valuta = tablePay->item(i, 3)->text();
		QString name = tablePay->item(i, 4)->text();
		QString section = tablePay->item(i, 6)->text();
		
		// Payment QIF
		qif.append("\nD" + valuta);
		qif.append("\nT" + amount);
		qif.append("\nP"+ settings.value("qif/payee_label", tr("Payment: ")).toString() + name + "("+member+")");
		qif.append("\nN" + ref);
		qif.append("\nM"+ settings.value("qif/memo", tr("Member UID: ")).toString() + member);
		qif.append("\nL" + settings.value("qif/account_income", "Membership Fee").toString());
		qif.append("\n^\n");
		
		// Update the Database
		query.bindValue(":date", valuta);
		query.bindValue(":amount", amount.toFloat());
		query.bindValue(":ref", ref);
		query.bindValue(":member", member);
		query.exec();
		if (query.lastError().type() != QSqlError::NoError) {
			qDebug() << query.lastQuery();
			qDebug() << query.lastError();
		}
	}
	
	// Set Invoices as paid
	query.prepare("UPDATE pps_invoice SET state=" + QString::number(Invoice::StatePaid) + " WHERE amount<=amount_paid;");
	query.exec();
	
	// Safe the QIF
	fileName = QFileDialog::getSaveFileName(this, tr("Save QIF File"), "", tr("Quicken Interchange (*.qif);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << qif;
		}
	}
}

