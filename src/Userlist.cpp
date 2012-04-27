
#include "Userlist.h"
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
#include <QDebug>

Userlist::Userlist(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(buttonSearch, SIGNAL(clicked()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(selectSection, SIGNAL(currentIndexChanged(QString)), this, SLOT(filterSection(QString)));
	connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showDetails(QModelIndex)));
	
	openDatabase();
	loadSections();
	loadData();
}
Userlist::~Userlist() {
	db.close();
}

void Userlist::openDatabase() {
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

void Userlist::loadData() {
	// Create Persons Database Tables if needed
	PPSPerson::createTables(db);
	Invoice::createTables(db);
	
	QSqlQuery query("SELECT uid,nickname,givenname,familyname,joining,section FROM ldap_persons;", db);
	tableModel->setQuery(query);
	
	// InvoiceState: o_pen, c_anceled, p_aid, u_nknown
	// Type: amount_f_ee, u_nknown
	// MemberState: m_ember, l_eave, i_nactive
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("UID"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Nickname"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Given name"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Family name"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("Joining Date"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("Section"));
	
	tableView->setSortingEnabled(true);
	tableView->setModel(tableModel);
}

void Userlist::loadSections() {
	selectSection->addItem(tr("All"), QVariant(""));
	QSqlQuery query("SELECT section FROM ldap_persons GROUP BY section;");
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	while (query.next()) {
		selectSection->addItem(query.value(0).toString(), QVariant(query.value(0).toString()));
	}
}

void Userlist::timerEvent(QTimerEvent *event) {
	killTimer(searchTimer);
	searchData();
}

void Userlist::searchData() {
	filterSection(selectSection->currentText());
}

void Userlist::searchDataTimeout(QString data) {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void Userlist::filterSection(QString section) {
	QString search;
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	for (int i = 0; i < sl.size(); i++) {
		if (i > 0) {
			search.append(" AND ");
		}
		search.append(QString("(nickname LIKE :p1_").append(QString::number(i)).append(" OR familyname LIKE :p2_").append(QString::number(i)).append(" OR givenname LIKE :p3_").append(QString::number(i)).append(")"));
	}
	
	QSqlQuery query(db);
	QString qs;
	bool bindSection = FALSE;
	if (section == "All") {
		qs = "SELECT uid,nickname,givenname,familyname,joining,section FROM ldap_persons";
		if (sl.size() > 0) {
			qs.append(" WHERE ").append(search);
		}
	} else {
		bindSection = TRUE;
		qs = "SELECT uid,nickname,givenname,familyname,joining,section FROM ldap_persons WHERE section=:section";
		if (sl.size() > 0) {
			qs.append(" AND (").append(search).append(")");
		}
	}
	
	query.prepare(qs);
	if (bindSection) {
		query.bindValue(":section", section);
	}
	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":p1_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p2_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":p3_").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
	}
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	tableModel->setQuery(query);
}

void Userlist::showDetails(QModelIndex index) {
	if (!index.isValid()) {
		return;
	}
	int uid = tableModel->record(index.row()).value(0).toInt();
	PPSPerson person;
	person.load(db, uid);
	
	detailUid->setText(QString::number(person.uid()));
	detailNickname->setText(person.nickname());
	switch (person.gender()) {
		case PPSPerson::GenderMale: detailGender->setText(tr("Male")); break;
		case PPSPerson::GenderFemale: detailGender->setText(tr("Female")); break;
		case PPSPerson::GenderBoth: detailGender->setText(tr("Both")); break;
		case PPSPerson::GenderNone: detailGender->setText(tr("None")); break;
		default: detailGender->setText(tr("Unknown")); break;
	}
	detailName->setText(person.familyName()+" "+person.givenName());
	detailAddress->setText(person.street());
	detailZipCity->setText(person.postalCode()+" "+person.city());
	detailCountry->setText(person.country());
	detailBirthday->setText(person.birthdate().toString("yyyy-MM-dd"));
	detailJoined->setText(person.joining().toString("yyyy-MM-dd"));
	switch (person.language()) {
		case PPSPerson::DE: detailLanguage->setText(tr("German")); break;
		case PPSPerson::EN: detailLanguage->setText(tr("English")); break;
		case PPSPerson::IT: detailLanguage->setText(tr("Italian")); break;
		case PPSPerson::FR: detailLanguage->setText(tr("French")); break;
		default: detailLanguage->setText(tr("English")); break;
	}
	detailSection->setText(person.section());
	switch (person.contributionClass()) {
		case PPSPerson::ContributeStudent: detailContributionClass->setText(tr("Student")); break;
		default: detailContributionClass->setText(tr("Full")); break;
	}
	
	QString s;
	QList<QString> l = person.telephone();
	for (int i = 0; i < l.size(); i++) {
		s.append(l.at(i) + "\n");
	}
	l = person.mobile();
	for (int i = 0; i < l.size(); i++) {
		s.append(l.at(i) + "\n");
	}
	detailTelephone->setText(s.trimmed());
	
	s = "";
	l = person.email();
	for (int i = 0; i < l.size(); i++) {
		s.append(l.at(i) + "\n");
	}
	detailEmail->setText(s.trimmed());
	
	Invoice *invoice = person.getInvoice();
	detailPaidDate->setText(invoice->paidDate().toString("yyyy-MM-dd"));
	
	//delete invoice;
}

