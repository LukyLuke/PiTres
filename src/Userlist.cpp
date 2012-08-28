/*
	Show a List with all Users - Payments can be executed (PaymentWizard)
	Copyright (C) 2012  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Userlist.h"
#include "data/Person.h"
#include "data/Invoice.h"
#include "PaymentWizard.h"

#include <QSizePolicy>
#include <QWidget>
#include <QTableView>
#include <QModelIndexList>
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
#include <QFileDialog>
#include <QRegExp>
#include <QPoint>
#include <QMenu>
#include <QDebug>

Userlist::Userlist(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(selectSection, SIGNAL(currentIndexChanged(QString)), this, SLOT(filterSection(QString)));
	connect(tableView, SIGNAL(activated(QModelIndex)), this, SLOT(showDetails(QModelIndex)));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	
	tableModel = new QSqlQueryModel(tableView);
	
	createContextMenu();
	loadSections();
	loadData();
	
	// Enable the ContextMenu
	tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTableContextMenu(const QPoint&)));
	
	editDateDialog = new QDialog(this);
	dateForm.setupUi(editDateDialog);
	dateForm.hintLabel->setText(tr("Select the new 'Member Due'-Date for the selected member."));
	connect(dateForm.actionChoose, SIGNAL(triggered()), this, SLOT(adjustMemberDueDate()));
}

Userlist::~Userlist() {
	delete actionManualPayment;
	delete actionEditDueDate;
}

void Userlist::createContextMenu() {
	actionManualPayment = new QAction(tr("&Pay Memberfee"), this);
	actionManualPayment->setShortcut(QKeySequence::Print);
	actionManualPayment->setStatusTip(tr("User pays current Memberfee cash"));
	connect(actionManualPayment, SIGNAL(triggered()), this, SLOT(payMemberfeeCash()));
	
	actionEditDueDate = new QAction(tr("&Change Member due Date"), this);
	actionEditDueDate->setShortcut(QKeySequence("CTRL + D"));
	actionEditDueDate->setStatusTip(tr("Change the 'Member due' Date from the current User"));
	connect(actionEditDueDate, SIGNAL(triggered()), this, SLOT(showMemberDueAdjust()));
}

void Userlist::loadData() {
	QSqlQuery query("SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname, ldap_persons.familyname, ldap_persons.city, ldap_persons.joining, ldap_persons.section FROM ldap_persons LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) GROUP BY ldap_persons_dates.uid;", db);
	tableModel->setQuery(query);
	
	// InvoiceState: o_pen, c_anceled, p_aid, u_nknown
	// Type: amount_f_ee, u_nknown
	// MemberState: m_ember, l_eave, i_nactive
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("UID"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Due"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Nickname"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Given name"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("Family name"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("City"));
	tableModel->setHeaderData(6, Qt::Horizontal, tr("Joining Date"));
	tableModel->setHeaderData(7, Qt::Horizontal, tr("Section"));
	
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

QSqlQuery Userlist::createQuery() {
	QString section = selectSection->currentText();
	QString search;
	QStringList sl = searchEdit->text().split(QRegExp("\\s+"), QString::SkipEmptyParts);
	for (int i = 0; i < sl.size(); i++) {
		if (i > 0) {
			search.append(" AND ");
		}
		search.append(QString("(ldap_persons.uid=:uid").append(QString::number(i)).append(" OR ldap_persons.nickname LIKE :nick").append(QString::number(i)).append(" OR ldap_persons.familyname LIKE :family").append(QString::number(i)).append(" OR ldap_persons.givenname LIKE :given").append(QString::number(i)).append(")"));
	}
	
	QSqlQuery query(db);
	QString qs;
	bool bindSection = (section != "All");
	if (!bindSection) {
		qs = "SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname, ldap_persons.familyname, ldap_persons.city, ldap_persons.joining, ldap_persons.section FROM ldap_persons LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid)";
		if (sl.size() > 0) {
			qs.append(" WHERE ").append(search);
		}
		qs.append(" GROUP BY ldap_persons_dates.uid;");
		
	} else {
		qs = "SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname, ldap_persons.familyname, ldap_persons.city, ldap_persons.joining, ldap_persons.section FROM ldap_persons LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) WHERE ldap_persons.section=:section";
		if (sl.size() > 0) {
			qs.append(" AND (").append(search).append(")");
		}
		qs.append(" GROUP BY ldap_persons_dates.uid;");
	}
	
	query.prepare(qs);
	if (bindSection) {
		query.bindValue(":section", section);
	}
	for (int i = 0; i < sl.size(); i++) {
		query.bindValue(QString(":uid").append(QString::number(i)), sl.at(i));
		query.bindValue(QString(":nick").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":family").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
		query.bindValue(QString(":given").append(QString::number(i)), QString("%%1%").arg(sl.at(i)));
	}
	return query;
}

void Userlist::filterSection(QString section) {
	QSqlQuery query = createQuery();
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
	showDetails(index.row());
}

void Userlist::showDetails(int index) {
	int uid = tableModel->record(index).value(0).toInt();
	PPSPerson person;
	person.load(uid);
	
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
		case PPSPerson::ContributeStudent: detailContributionClass->setText(tr("Limited")); break;
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
	
	//Invoice *invoice = person.getInvoice();
	detailMemberDue->setText(person.paidDue().toString("yyyy-MM-dd"));
	
	//delete invoice;
}

void Userlist::exportData() {
	QSqlQuery query = createQuery();
	query.exec();
	
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	
	QRegExp re("[\"',\r\n]+");
	QString csv("Member,Nickname,Givenname,Familyname,City,Section,Paid\n");
	while (query.next()) {
		// "SELECT uid,paid_due_date,nickname,givenname,familyname,city,joining,section FROM ldap_persons;";
		csv.append( query.value(0).toString().remove(re) ).append(",");
		csv.append( query.value(2).toString().remove(re) ).append(",");
		csv.append( query.value(3).toString().remove(re) ).append(",");
		csv.append( query.value(4).toString().remove(re) ).append(",");
		csv.append( query.value(5).toString().remove(re) ).append(",");
		csv.append( query.value(7).toString().remove(re) ).append(",");
		csv.append( query.value(1).toString().remove(re) ).append("\n");
	}
	
	// Safe as File
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save the Userlist"), "", tr("CSV (*.csv);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << csv;
		}
	}
}

void Userlist::showTableContextMenu(const QPoint &point) {
	QMenu menu(tableView);
	menu.addAction(actionManualPayment);
	menu.addAction(actionEditDueDate);
	menu.exec(tableView->mapToGlobal(point));
}

int Userlist::getFirstSelectedUid() {
	QModelIndexList selection = tableView->selectionModel()->selectedIndexes();
	if (selection.size() > 0) {
		int row = selection.at(0).row();
		return tableModel->record(row).value(0).toInt();
	}
	return 0;
}

void Userlist::payMemberfeeCash() {
	PaymentWizard *wizard = new PaymentWizard(this);
	int uid = getFirstSelectedUid();
	if (uid > 0) {
		wizard->setPerson(uid);
		wizard->show();
	}
}

void Userlist::showMemberDueAdjust() {
	QSettings settings;
	int uid = getFirstSelectedUid();
	if (uid > 0) {
		PPSPerson person;
		person.load(uid);
		if (person.paidDue() > QDate(1970, 1, 2)) {
			dateForm.fromDate->setDate(person.paidDue());
		} else {
			dateForm.fromDate->setDate( QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-12-31").toString()), "yyyy-MM-dd") );
		}
		editDateDialog->show();
	}
}

void Userlist::adjustMemberDueDate() {
	QSettings settings;
	int uid = getFirstSelectedUid();
	if (uid > 0) {
		PPSPerson person;
		person.load(uid);
		person.setPaidDue(dateForm.fromDate->date());
		person.save();
	}
	editDateDialog->hide();
}

