
#include "Userlist.h"

#include <QSizePolicy>
#include <QWidget>
#include <QTableView>
#include <QVariant>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

Userlist::Userlist(QWidget *parent) {
	setupUi(this);
	loadData();
}
Userlist::~Userlist() {
	db.close();
}

void Userlist::loadData() {
	//storage = new SqliteStorage("data/userlist.sqlite", "users");
	QFileInfo dbfile("data/userlist.sqlite");
	qDebug() << "Loading DB: " + dbfile.absoluteFilePath();
	
	if (!dbfile.absoluteDir().exists()) {
		dbfile.absoluteDir().mkpath(dbfile.absolutePath());
		qDebug() << "Path did not exists, created: " + dbfile.absolutePath();
	}
	
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(dbfile.absoluteFilePath());
	db.open();
	
	QSqlRelationalTableModel *tableModel = new QSqlRelationalTableModel(tableView, db);
	tableModel->setTable("invoices");
	tableModel->select();
	
	// InvoiceState: o_pen, c_anceled, p_aid, u_nknown
	// Type: amount_f_ee, u_nknown
	// MemberState: m_ember, l_eave, i_nactive
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("UID"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Reference"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Order Date"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Payable due"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("Paid date"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(6, Qt::Horizontal, tr("State"));
	tableModel->setHeaderData(7, Qt::Horizontal, tr("Invoice Type"));
	tableModel->setHeaderData(8, Qt::Horizontal, tr("Nickname"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Firstname"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Lastname"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Street"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("PLZ"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("City"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Country"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Email"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Language"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Sex"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Section"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("To pay"));
	tableModel->setHeaderData(9, Qt::Horizontal, tr("Member State"));
	tableView->setModel(tableModel);
}


