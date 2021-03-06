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

Userlist::Userlist(QWidget *parent) : QWidget(parent) {
	QSettings settings;
	setupUi(this);

	connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(searchData()));
	connect(searchEdit, SIGNAL(textChanged(QString)), this, SLOT(searchDataTimeout(QString)));
	connect(selectSection, SIGNAL(currentIndexChanged(QString)), this, SLOT(filterSection(QString)));
	connect(tableView, SIGNAL(activated(QModelIndex)), this, SLOT(showDetails(QModelIndex)));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	connect(btnLdap, SIGNAL(clicked()), this, SLOT(exportLdif()));
	connect(btnFindMissing, SIGNAL(clicked()), this, SLOT(openMissingDialog()));

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
	
	missingPeopleDialog = new QDialog(this);
	missingPeopleForm.setupUi(missingPeopleDialog);
	QDate paidDue = QDate::currentDate();
	if (settings.value("invoice/member_due_next_year", TRUE).toBool()) {
		paidDue = paidDue.addYears(1);
	}
	missingPeopleForm.datePaidDue->setDate( QDate::fromString(paidDue.toString(settings.value("invoice/member_due_format", "yyyy-02-15").toString()), "yyyy-MM-dd") );
	missingPeopleForm.tableView->setModel(new QSqlQueryModel);
	missingPeopleForm.tableView->model()->setHeaderData(0, Qt::Horizontal, tr("UID"));
	missingPeopleForm.tableView->model()->setHeaderData(1, Qt::Horizontal, tr("Name"));
	missingPeopleForm.tableView->model()->setHeaderData(2, Qt::Horizontal, tr("Address"));
	missingPeopleForm.tableView->model()->setHeaderData(3, Qt::Horizontal, tr("city"));
	missingPeopleForm.tableView->model()->setHeaderData(4, Qt::Horizontal, tr("Section"));
	missingPeopleForm.tableView->model()->setHeaderData(5, Qt::Horizontal, tr("Last paid"));
	missingPeopleForm.tableView->model()->setHeaderData(6, Qt::Horizontal, tr("Paid due"));
	connect(missingPeopleForm.btnClose, SIGNAL(clicked()), missingPeopleDialog, SLOT(hide()));
	connect(missingPeopleForm.btnExportLdif, SIGNAL(clicked()), this, SLOT(exportMissingPeople()));
	connect(missingPeopleForm.datePaidDue, SIGNAL(dateChanged(QDate)), this, SLOT(openMissingDialog()));
}

Userlist::~Userlist() {
	delete actionManualPayment;
	delete actionEditDueDate;
	delete editDateDialog;
	delete missingPeopleDialog;
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
	QSqlQuery query("SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname,"
		" ldap_persons.familyname, ldap_persons.city, ldap_persons.joining, ldap_persons.section FROM ldap_persons"
		" LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) GROUP BY ldap_persons_dates.uid;", db);
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
	tableModel->setHeaderData(8, Qt::Horizontal, tr("Type"));

	tableView->setSortingEnabled(true);
	tableView->setModel(tableModel);
}

void Userlist::loadSections() {
	selectSection->addItem(tr("All"), QVariant(""));
	QSqlQuery query("SELECT name FROM pps_sections ORDER BY name;", db);
	while (query.next()) {
		selectSection->addItem(query.value(0).toString(), query.value(0));
	}
}

void Userlist::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	searchData();
}

void Userlist::searchData() {
	filterSection(selectSection->currentText());
}

void Userlist::searchDataTimeout(QString /*data*/) {
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
		search.append(QString("(ldap_persons.uid=:uid").append(QString::number(i))
			.append(" OR ldap_persons.nickname LIKE :nick").append(QString::number(i))
			.append(" OR ldap_persons.familyname LIKE :family").append(QString::number(i))
			.append(" OR ldap_persons.givenname LIKE :given").append(QString::number(i)).append(")")
		);
	}

	QSqlQuery query(db);
	QString qs;
	bool bindSection = (selectSection->currentIndex() != 0);
	if (!bindSection) {
		qs = "SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname, ldap_persons.familyname,"
			 " ldap_persons.city, ldap_persons.joining, ldap_persons.section, ldap_persons.type FROM ldap_persons LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid)";
		if (sl.size() > 0) {
			qs.append(" WHERE ").append(search);
		}
		qs.append(" GROUP BY ldap_persons_dates.uid;");

	} else {
		qs = "SELECT ldap_persons.uid, MAX(ldap_persons_dates.paid_due) AS paid_due, ldap_persons.nickname, ldap_persons.givenname, ldap_persons.familyname,"
			" ldap_persons.city, ldap_persons.joining, ldap_persons.section, ldap_persons.type FROM ldap_persons LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid)"
			" WHERE ldap_persons.section=:section";
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

void Userlist::filterSection(QString /*section*/) {
	QSqlQuery query = createQuery();
	query.exec();
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
	detailName->setText(person.familyName()+" "+person.givenName());
	detailAddress->setText(person.street());
	detailZipCity->setText(person.postalCode()+" "+person.city());
	detailCountry->setText(person.country());
	detailBirthday->setText(person.birthdate().toString("yyyy-MM-dd"));
	detailJoined->setText(person.joining().toString("yyyy-MM-dd"));
	detailSection->setText(person.section());

	switch (person.gender()) {
		case PPSPerson::GenderMale: detailGender->setText(tr("Male")); break;
		case PPSPerson::GenderFemale: detailGender->setText(tr("Female")); break;
		case PPSPerson::GenderBoth: detailGender->setText(tr("Both")); break;
		case PPSPerson::GenderNone: detailGender->setText(tr("None")); break;
		default: detailGender->setText(tr("Unknown")); break;
	}

	switch (person.type()) {
		case PPSPerson::LandLubber: detailType->setText(tr("LandLubber")); break;
		case PPSPerson::Sympathizer: detailType->setText(tr("Sympathizer")); break;
		case PPSPerson::Pirate: detailType->setText(tr("Pirate")); break;
		case PPSPerson::Veteran: detailType->setText(tr("Veteran")); break;
		case PPSPerson::PlankWalker: detailType->setText(tr("PlankWalker")); break;
		case PPSPerson::FleetPirate: detailType->setText(tr("FleetPirate")); break;
		default: detailType->setText(tr("Unknown")); break;
	}

	switch (person.language()) {
		case PPSPerson::DE: detailLanguage->setText(tr("German")); break;
		case PPSPerson::EN: detailLanguage->setText(tr("English")); break;
		case PPSPerson::IT: detailLanguage->setText(tr("Italian")); break;
		case PPSPerson::FR: detailLanguage->setText(tr("French")); break;
		default: detailLanguage->setText(tr("English")); break;
	}
#ifndef FIO
	switch (person.contributionClass()) {
		case PPSPerson::ContributeStudent: detailContribute->setText(tr("Limited")); break;
		default: detailContribute->setText(tr("Full")); break;
	}
#else
	detailContribute->hide();
	labelContribute->hide();
#endif

	switch (person.notify()) {
		case PPSPerson::SnailMail: detailNotifyType->setText(tr("Snail-Mail")); break;
		case PPSPerson::EMail: detailNotifyType->setText(tr("E-Mail")); break;
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
	PPSPerson person;
	
	// Reset all membes types
	PPSPerson::resetMembertypes();

	// Create the query and export all data
	QSqlQuery query = createQuery();
	query.exec();

	QRegExp re("[\"',\r\n]+");
	QString csv("Member,Nickname,Givenname,Familyname,Postalcode,City,Country,Section,Type,Paid,Email 1,Email 2,Tel 1,Tel 2\n");
	
	// "SELECT uid,paid_due_date,nickname,givenname,familyname,city,joining,section FROM ldap_persons;";
	while (query.next()) {
		person.load(query.value(0).toInt());
		
		csv.append( QString::number(person.uid()) ).append(",");
		csv.append( person.nickname().remove(re) ).append(",");
		csv.append( person.givenName().remove(re) ).append(",");
		csv.append( person.familyName().remove(re) ).append(",");
		csv.append( person.postalCode().remove(re) ).append(",");
		csv.append( person.city().remove(re) ).append(",");
		csv.append( person.country().remove(re) ).append(",");
		csv.append( person.section().remove(re) ).append(",");
		switch (person.type()) {
			case PPSPerson::LandLubber: csv.append("LandLubber").append(","); break;
			case PPSPerson::Sympathizer: csv.append("Sympathizer").append(","); break;
			case PPSPerson::Pirate: csv.append("Pirate").append(","); break;
			case PPSPerson::Veteran: csv.append("Veteran").append(","); break;
			case PPSPerson::PlankWalker: csv.append("PlankWalker").append(","); break;
			case PPSPerson::FleetPirate: csv.append("FleetPirate").append(","); break;
			default: csv.append("Unknown").append(","); break;
		}
		
		// Paid until
		csv.append( person.paidDue().toString("yyyy-MM-dd").append(",") );
		
		// Email and Telephone
		QString val;
		val = person.email().value(0);
		csv.append( val.remove(re).append(",") );
		
		val = person.email().value(1);
		csv.append( val.remove(re).append(",") );
		
		val = person.telephone().value(0);
		csv.append( val.remove(re).append(",") );
		
		val = person.telephone().value(1);
		csv.append( val.remove(re) );
		
		csv.append("\n");
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
			QDate paidDue = QDate::fromString(QDate::currentDate().toString(settings.value("invoice/member_due_format", "yyyy-02-15").toString()), "yyyy-MM-dd");
			if (settings.value("invoice/member_due_next_year", TRUE).toBool()) {
				paidDue = paidDue.addYears(1);
			}
			dateForm.fromDate->setDate(paidDue);
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

void Userlist::exportLdif() {
	// using ldap_modify:
	// ldapmodify -D "uniqueIdentifier=XX,dc=members,st=XX,dc=piratenpartei,dc=ch" -W -h localhost -p 1389 -c -f XX.ldif &> import.log
	QSettings settings;
	QHash<QString, LdifData> hashList;
	QHash<QString, QString> sections;
	QString ldif = "";
	QString section, member, duedate, type, subsection, value, save_section;
	QDate due;
	qint32 cnt = 0;
	QString members_dn = settings.value("ldif/members_dn", "uniqueIdentifier=%1,dc=members,dc=piratenpartei,dc=ch").toString();
	QString main_dn = settings.value("ldif/main_dn", "uniqueIdentifier=%1,dc=members,%3st=%2,dc=piratenpartei,dc=ch").toString();
	QString sub_concat = settings.value("ldif/subsection_concat", "l=%1,").toString();
	QString attribute = settings.value("ldif/memberstate_attribute", "ppsVotingRightUntil").toString();
	QString membertype = settings.value("ldif/membertype_attribute", "employeeType").toString();
	bool replaceAttribute = settings.value("ldif/replace_attribute", false).toBool();
	QSqlQuery query(db);

	// Reset all membes types
	PPSPerson::resetMembertypes();

	// Only get the last if entries should be relaced or all the other way
	if (replaceAttribute) {
		query.prepare("SELECT ldap_persons.uid,MAX(ldap_persons_dates.paid_due) AS paid_due,ldap_persons.type,ldap_persons.section FROM ldap_persons"
			" LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) GROUP BY ldap_persons_dates.uid;");
	} else {
		query.prepare("SELECT ldap_persons.uid,ldap_persons_dates.paid_due,ldap_persons.type,ldap_persons.section FROM ldap_persons"
			" LEFT JOIN ldap_persons_dates ON (ldap_persons.uid = ldap_persons_dates.uid) ORDER BY ldap_persons_dates.uid,ldap_persons_dates.paid_due;");
	}

	if (query.exec()) {
		while (query.next()) {
			member = query.value(0).toString();
			duedate = QDate::fromString(query.value(1).toString(), "yyyy-MM-dd").toString("yyyyMMdd235959Z");
			type = query.value(2).toString();
			section = query.value(3).toString();

			// If we want to add all paid dates and not only the last, we create an entry only the first time and add just the date if we get the user again
			LdifData d;
			if (!hashList.contains(member)) {
				d.section = section;
				d.uid = member;
				d.type = type;
				hashList.insert(member, d);
			}

			// Add the current date if it is not already in
			d = hashList.value(member);
			if (!d.dates.contains(duedate)) {
				d.dates.append(duedate);
			}
			hashList.insert(member, d);
		}
	} else {
		QMessageBox::warning(this, tr("Adjusting dates failed"), tr("An Error occured while getting Userdata and Paiddue-Dates from Database:\n\n%1").arg(query.lastError().text()));
		return;
	}

	// Load all Sections in a Hash-Cache
	Section::getNameParentHash(&sections);

	// Create the LDIF
	QHash<QString, LdifData>::const_iterator it = hashList.constBegin();
	while (it != hashList.constEnd()) {
		LdifData d = it.value();

		if (d.section.isEmpty() || !sections.contains(d.section)) {
			ldif.append("dn: " + members_dn.arg(d.uid));

		} else {
			// Create the SubSection String if needed
			section = d.section;
			subsection = "";
			cnt = 0;
			do {
				save_section = section;
				value = sections.value(section);
				if (sections.contains(value)) {
					subsection.append(sub_concat.arg(section));
					section = value;
				} else {
					section = save_section;
					break;
				}
			} while (++cnt < sections.size());

			ldif.append("dn: " + main_dn.arg(d.uid, section, subsection));
		}
		ldif.append("\nchangetype: modify");

		//ldif.append("\ndelete: " + attribute);
		//ldif.append("\n-");
		//ldif.append("\nadd: " + attribute);

		// Add the employee type
		ldif.append("\nreplace: " + membertype + "\n");
		ldif.append(membertype + ": " + d.type + "\n-");

		// Add all dates
		ldif.append("\nreplace: " + attribute + "\n");
		for (int i = 0; i < d.dates.size(); ++i) {
			ldif.append(attribute).append(": ").append(d.dates.at(i));
			ldif.append("\n");
		}
		ldif.append("-\n\n");

		++it;
	}

	// Save the LDIF
	if (ldif.length() > 0) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save LDIF File"), "", tr("LDIF (*.ldif);;Plaintext (*.txt)"));
		if (!fileName.isEmpty()) {
			QFile f(fileName);
			if (f.open(QFile::WriteOnly | QFile::Truncate)) {
				QTextStream out(&f);
				out << ldif;
			} else {
				QMessageBox::warning(this, tr("Adjusting dates failed"), tr("An Error occured while opening the LDIF-File.\n\nDo you have Write-Permission to\n%1?").arg(fileName));
			}
		}
	} else {
		QMessageBox::warning(this, tr("Adjusting dates failed"), tr("No Datasets found which would need an Update..."));
	}
}

void Userlist::openMissingDialog() {
	QSqlQuery query(db);
	query.prepare("SELECT pps_invoice.member_uid, pps_invoice.address_name, pps_invoice.address_street1, pps_invoice.address_city,"
		" pps_invoice.for_section, pps_invoice.paid_date, ldap_persons_dates.paid_due"
		" FROM pps_invoice"
		" LEFT JOIN ldap_persons_dates ON (pps_invoice.member_uid=ldap_persons_dates.uid)"
		" LEFT JOIN ldap_persons ON (pps_invoice.member_uid=ldap_persons.uid)"
		" WHERE (pps_invoice.state==2 OR pps_invoice.state==4) AND ldap_persons.uid IS NULL"
		" AND ldap_persons_dates.paid_due>=:paid_due"
		" GROUP BY pps_invoice.member_uid"
		" ORDER BY pps_invoice.member_uid ASC, ldap_persons_dates.paid_due ASC;");
	query.bindValue(":paid_due", missingPeopleForm.datePaidDue->date());
	query.exec();
	QSqlQueryModel *model = (QSqlQueryModel*) missingPeopleForm.tableView->model();
	model->setQuery(query);
	if (!missingPeopleDialog->isVisible()) {
		missingPeopleDialog->show();
	}
}

void Userlist::exportMissingPeople() {
	QSettings settings;
	QString list;
	QSqlQueryModel *model = (QSqlQueryModel *) missingPeopleForm.tableView->model();

	while (model->canFetchMore()) {
		model->fetchMore();
	}

	for (int i = 0; i < model->rowCount(); i++) {
		QModelIndex idx = model->index(i, 0);
		list.append(model->data(idx, Qt::DisplayRole).toString()).append("\n");
	}

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save discrepancies UIDs"), "", tr("Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << list;
		} else {
			QMessageBox::warning(this, tr("Export failed"), tr("An Error occured while opening the file.\n\nDo you have Write-Permission to\n%1?").arg(fileName));
		}
	}

	missingPeopleDialog->hide();
}
