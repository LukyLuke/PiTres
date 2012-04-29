
#include "SentBills.h"
#include "data/Person.h"
#include "data/Invoice.h"

#include <QSizePolicy>
#include <QWidget>
#include <QTableView>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSettings>
#include <QModelIndex>
#include <QVariant>
#include <QSqlRecord>
#include <QPoint>
#include <QMenu>
#include <QDebug>


SentBills::SentBills(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(buttonSearch, SIGNAL(clicked()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showDetails(QModelIndex)));
	
	openDatabase();
	loadData();
}

SentBills::~SentBills() {
	// dd.close();
}

void SentBills::openDatabase() {
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
	
	tableModel = new QSqlQueryModel(tableView);
}

void SentBills::loadData() {
	// Create Persons Database Tables if needed
	PPSPerson::createTables(db);
	Invoice::createTables(db);
	
	QSqlQuery query("SELECT * FROM pps_invoice;", db);
	tableModel->setQuery(query);
	
	tableModel->setHeaderData(0,  Qt::Horizontal, tr("UID"));
	tableModel->setHeaderData(1,  Qt::Horizontal, tr("Reference"));
	tableModel->setHeaderData(2,  Qt::Horizontal, tr("Issue Date"));
	tableModel->setHeaderData(3,  Qt::Horizontal, tr("Payable Date"));
	tableModel->setHeaderData(4,  Qt::Horizontal, tr("Paid Date"));
	tableModel->setHeaderData(5,  Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(6,  Qt::Horizontal, tr("Amount Paid"));
	tableModel->setHeaderData(7,  Qt::Horizontal, tr("State"));
	tableModel->setHeaderData(8,  Qt::Horizontal, tr("Prefix"));
	tableModel->setHeaderData(9,  Qt::Horizontal, tr("Company"));
	tableModel->setHeaderData(10, Qt::Horizontal, tr("Name"));
	tableModel->setHeaderData(11, Qt::Horizontal, tr("Address"));
	tableModel->setHeaderData(12, Qt::Horizontal, tr("Address 2"));
	tableModel->setHeaderData(13, Qt::Horizontal, tr("PLZ, City"));
	tableModel->setHeaderData(14, Qt::Horizontal, tr("Email"));
	tableModel->setHeaderData(15, Qt::Horizontal, tr("Section"));
	
	tableView->setSortingEnabled(true);
	tableView->setModel(tableModel);
}

void SentBills::timerEvent(QTimerEvent *event) {
	killTimer(searchTimer);
	searchData();
}

void SentBills::searchDataTimeout(QString data) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void SentBills::searchData() {
	QSqlQuery query(db);
	QString qs("SELECT * FROM pps_invoice");
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	
	if (sl.size() > 0) {
		qs.append(" WHERE ");
		for (int i = 0; i < sl.size(); i++) {
			if (i > 0) {
				qs.append(" AND ");
			}
			qs.append(QString("(reference LIKE :p1_").append(QString::number(i)).append(" OR member_uid LIKE :p2_").append(QString::number(i)));
			qs.append(QString(" OR issue_date LIKE :p3_").append(QString::number(i)).append(" OR payable_date LIKE :p4_").append(QString::number(i)));
			qs.append(QString(" OR address_name LIKE :p5_").append(QString::number(i)).append(" OR address_email LIKE :p6_").append(QString::number(i)));
			qs.append(")");
		}
	}
	qs.append(" ORDER BY issue_date,paid_date,state ASC");
	query.prepare(qs);

	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":p1_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p2_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p3_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p4_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p5_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p6_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
	}
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	tableModel->setQuery(query);
}