/*
	The Donaitions Widget is for managing all Donaitions
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

#include "Donations.h"

Donations::Donations(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(btnImportBack, SIGNAL(clicked()), this, SLOT(wizardBack()));
	connect(btnImportNext, SIGNAL(clicked()), this, SLOT(wizardNext()));
	connect(btnImportSave, SIGNAL(clicked()), this, SLOT(wizardNext()));
	
	connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(dateUntil, SIGNAL(dateChanged(QDate)), this, SLOT(searchData()));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	connect(btnEmail, SIGNAL(clicked()), this, SLOT(sendEmail()));
	connect(btnChooseDonationImport, SIGNAL(clicked()), this, SLOT(showFileDialog()));
	
	connect(sectionList, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSection()));
	connect(yearList, SIGNAL(itemSelectionChanged()), this, SLOT(selectYear()));
	
	tableModel = new QSqlQueryModel(tableView);
	donationsModel = new QSqlQueryModel(donationsTable);
	
	fileImport = false;
	
	loadData();
	loadSectionDonations();
	enableWizardButtons();
}

Donations::~Donations() {}

void Donations::loadData() {
	dateFrom->setDate(QDate::currentDate().addMonths(-4));
	dateUntil->setDate(QDate::currentDate());
	
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("Person"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Amount"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("City"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Section"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("Date"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("Objective"));
	
	tableView->setSortingEnabled(false);
	tableView->setModel(tableModel);
}

void Donations::loadSectionDonations() {
	QSqlQuery query = createDonationsQuery();
	query.exec();
	donationsModel->setQuery(query);
	
	donationsModel->setHeaderData(0, Qt::Horizontal, tr("Person"));
	donationsModel->setHeaderData(1, Qt::Horizontal, tr("Amount"));
	donationsModel->setHeaderData(2, Qt::Horizontal, tr("City"));
	donationsModel->setHeaderData(3, Qt::Horizontal, tr("Section"));
	donationsModel->setHeaderData(4, Qt::Horizontal, tr("Date"));
	donationsModel->setHeaderData(5, Qt::Horizontal, tr("Objective"));
	
	donationsTable->setSortingEnabled(false);
	donationsTable->setModel(donationsModel);
	
	QSqlQuery sections(db);
	sections.prepare("SELECT section FROM donation_entities GROUP BY section ORDER BY section ASC;");
	sections.exec();
	sectionList->addItem(tr("all"));
	while (sections.next()) {
		sectionList->addItem(sections.value(0).toString());
	}
}

void Donations::wizardBack() {
	if (stackedWidget->currentIndex() >= 0) {
		stackedWidget->setCurrentIndex(stackedWidget->currentIndex()-1);
	}
	enableWizardButtons();
}

void Donations::wizardNext() {
	if (stackedWidget->currentIndex() < (stackedWidget->count()-1)) {
		stackedWidget->setCurrentIndex(stackedWidget->currentIndex()+1);
	}
	enableWizardButtons();
}

void Donations::enableWizardButtons() {
	switch (stackedWidget->currentIndex()) {
		case 0: // init page
			btnImportBack->setEnabled(false);
			btnImportNext->setEnabled(true);
			btnImportSave->setEnabled(false);
			break;
		case 1: // preview
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(false);
			btnImportSave->setEnabled(true);
			
			wizardPrepareImport();
			break;
		case 2: // import/save
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(false);
			btnImportSave->setEnabled(false);
			
			wizardImport();
			break;
	}
}

void Donations::wizardPrepareImport() {
	fileImport = !(editImportFile->text().isEmpty());
	tablePreviewImport->clear();
	
	if (!fileImport) {
		tablePreviewImport->setRowCount(1);
		tablePreviewImport->setItem(0, 0,  new QTableWidgetItem(editMember->text()));
		tablePreviewImport->setItem(0, 1,  new QTableWidgetItem(editCompany->text()));
		tablePreviewImport->setItem(0, 2,  new QTableWidgetItem(editAddress->text()));
		tablePreviewImport->setItem(0, 3,  new QTableWidgetItem(editCity->text()));
		tablePreviewImport->setItem(0, 4,  new QTableWidgetItem(editCountry->text()));
		tablePreviewImport->setItem(0, 5,  new QTableWidgetItem(editPhone->text()));
		tablePreviewImport->setItem(0, 6,  new QTableWidgetItem(editMobile->text()));
		tablePreviewImport->setItem(0, 7,  new QTableWidgetItem(editEmail->text()));
		tablePreviewImport->setItem(0, 8,  new QTableWidgetItem(QString::number(editAmount->value())));
		tablePreviewImport->setItem(0, 9,  new QTableWidgetItem(editObjective->text()));
		tablePreviewImport->setItem(0, 10, new QTableWidgetItem(editMemo->text()));
		
	} else {
		QFile file(editImportFile->text());
		if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
			return;
		}
		
		QStringList list;
		QByteArray line;
		int row = 0;
		while (!file.atEnd()) {
			if (row == 0 && checkFirstLineHeader->checkState() == Qt::Checked) {
				row++;
				continue;
			}
			
			if (tablePreviewImport->rowCount() < row) {
				tablePreviewImport->insertRow(tablePreviewImport->rowCount()-1);
			}
			
			line = file.readLine();
			list = QString(line).trimmed().split(editSeparator->text());
			for (int col = 0; col < list.size(); col++) {
				tablePreviewImport->setItem(row, col, new QTableWidgetItem(list.at(col)));
			}
			row++;
		}
	}
}

void Donations::wizardImport() {
	
}

void Donations::showFileDialog() {
	editImportFile->setText(
		QFileDialog::getOpenFileName(this, tr("Load Donations-File"), "", tr("Comma seperated File (*.csv);;Quicken Interchange Format (*.qif)"))
	);
}

QSqlQuery Donations::createQuery() {
	QSqlQuery query(db);
	query.prepare("SELECT address_name,amount,address_city,section,booked,objective FROM donation_entities WHERE"
	              " booked>=:start AND booked<=:end AND contributed<=:contr ORDER BY section ASC;");
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	return query;
}

QSqlQuery Donations::createDonationsQuery() {
	QDate from, to;
	QSqlQuery query(db);
	
	if (yearList->selectedItems().count() > 0 && yearList->currentRow() > 0) {
		from.setDate(yearList->selectedItems().at(0)->text().toInt(), 01, 01);
		to.setDate(yearList->selectedItems().at(0)->text().toInt(), 12, 31);
	} else {
		from.setDate(1900, 01, 01);
		to = QDate::currentDate();
	}
	
	query.prepare(QString("SELECT address_name,amount,address_city,section,booked,objective FROM donation_entities WHERE"
	                      " section%1:section AND booked>=:start"
	                      " AND booked<=:end AND contributed<=:contr ORDER BY section ASC;").arg(
	                         (sectionList->currentIndex() == 0 || sectionList->count() == 0 ? "<>" : "=")
	                      ));
	query.bindValue(":start", from.toString("yyyy-MM-dd"));
	query.bindValue(":end", to.toString("yyyy-MM-dd"));
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":section", sectionList->currentText());
	return query;
}

void Donations::searchData() {
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	showOverview();
}

void Donations::searchDonations() {
	QSqlQuery query = createDonationsQuery();
	query.exec();
	donationsModel->setQuery(query);
	showDonationsDetails();
}

void Donations::showOverview() {
	QSettings settings;
	QSqlQuery query(db);
	
	// Clean the GridContainer
	QLayoutItem *child;
	while ((child = overviewLayout->takeAt(0)) != 0) {
		delete ((QLabel *)child->widget());
		delete child;
	}
	
	// Load "do not contribute this sections" list
	QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
	dontContribute.removeAll("");
	
	// Refill with Data
	query.prepare("SELECT SUM(amount/2) AS amount,section FROM donation_entities WHERE booked>=:start AND booked<=:end AND contributed<=:contr GROUP BY section;");
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	query.exec();
	
	double sum = 0.0;
	int row = 0;
	while (query.next()) {
		QLabel *s = new QLabel( tr("Section: %1").arg(query.value(1).toString()) );
		QLabel *a = new QLabel( tr("%1 sFr.").arg(query.value(0).toDouble(), 0, 'f', 2) );
		a->setAlignment(Qt::AlignRight);
		if (dontContribute.contains(query.value(1).toString())) {
			s->setStyleSheet("QLabel { color: darkRed }");
			a->setStyleSheet("QLabel { color: darkRed }");
		} else {
			sum += query.value(0).toDouble();
		}
		overviewLayout->addWidget(s, row, 0);
		overviewLayout->addWidget(a, row, 1);
		row++;
	}
	
	// Total
	QLabel *s = new QLabel( tr("<b>Total:</b>") );
	QLabel *a = new QLabel( tr("<b>%1 sFr.</b>").arg(sum, 0, 'f', 2) );
	a->setAlignment(Qt::AlignRight);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(a, row, 1);
}

void Donations::showDonationsDetails() {
	QSettings settings;
	QSqlQuery query = createDonationsQuery();
	query.exec();
	
	// Clean the GridContainer
	QLayoutItem *child;
	while ((child = detailsLayout->takeAt(0)) != 0) {
		delete ((QLabel *)child->widget());
		delete child;
	}
	
	// Refill with Data
	QHash<QString, double> sum;
	QHash<QString, int> num;
	QString section;
	while (query.next()) {
		section = query.value(4).toString();
		if (!sum.contains(section)) {
			sum.insert(section, 0);
			num.insert(section, 0);
		}
		num.find(section).value()++;
		sum.find(section).value() += query.value(1).toDouble() / 2;
	}
	
	int row = 0;
	int ctotal = 0;
	double stotal = 0.0;
	QHash<QString, double>::const_iterator iter = sum.constBegin();
	while (iter != sum.constEnd()) {
		QLabel *t = new QLabel( tr("<b>Section %1</b>").arg(iter.key()) );
		detailsLayout->addWidget(t, row++, 0);
		
		stotal += iter.value();
		ctotal += num.find(iter.key()).value();
		
		QLabel *kp = new QLabel( tr("Payments:") );
		QLabel *vp = new QLabel( QString("%1").arg(num.find(iter.key()).value()) );
		kp->setAlignment(Qt::AlignRight);
		vp->setAlignment(Qt::AlignRight);
		detailsLayout->addWidget(kp, row, 0);
		detailsLayout->addWidget(vp, row++, 1);
		
		QLabel *kt = new QLabel( tr("Total:") );
		QLabel *vt = new QLabel( QString("%1").arg(iter.value(), 0, 'f', 2) );
		kt->setAlignment(Qt::AlignRight);
		vt->setAlignment(Qt::AlignRight);
		detailsLayout->addWidget(kt, row, 0);
		detailsLayout->addWidget(vt, row++, 1);
		iter++;
	}
	
	// Totals
	QLabel *kp = new QLabel( tr("<b>Payments:</b>") );
	QLabel *vp = new QLabel( QString("<b>%1</b>").arg(ctotal) );
	kp->setAlignment(Qt::AlignLeft);
	vp->setAlignment(Qt::AlignRight);
	detailsLayout->addWidget(kp, row, 0);
	detailsLayout->addWidget(vp, row++, 1);
	
	QLabel *kt = new QLabel( tr("<b>Total:</b>") );
	QLabel *vt = new QLabel( QString("<b>%1</b>").arg(stotal, 0, 'f', 2) );
	kt->setAlignment(Qt::AlignLeft);
	vt->setAlignment(Qt::AlignRight);
	detailsLayout->addWidget(kt, row, 0);
	detailsLayout->addWidget(vt, row++, 1);
}

void Donations::createQif() {
	QSettings settings;
	QString valuta = QDate::currentDate().toString("yyyy-MM-dd");
	QString date = QDate::currentDate().toString("yyyyMMdd");
	QString memo = settings.value("contribution/memo", "Contribution: %1").toString();
	QString section, amount;
	QSqlQuery query2(db);
	QSqlQuery query(db);
	
	// Load "do not contribute this sections" list
	QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
	dontContribute.removeAll("");
	QString notIn = QString(",?").repeated(dontContribute.size());
	if (!dontContribute.empty()) {
		notIn.remove(0, 1).append(")").prepend("AND section NOT IN (");
	}
	
	// Initialize the QIF and the list with all Section-QIFs
	sectionQif.clear();
	QString qif_national("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	if (!memoText->text().isEmpty()) {
		memo = memoText->text();
	}
	
	// TODO: Try to make a Split-Transaction QIF here with all Donation-Memos from the Database
	
	// Get all Data
	query.prepare(QString("SELECT SUM(amount/2) AS amount,section FROM donation_entities WHERE booked>=:start AND booked<=:end AND contributed<=:contr %1 GROUP BY for_section;").arg(notIn));
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	for (int i = 0; i < dontContribute.size(); i++) {
		query.bindValue(i+3, dontContribute.at(i));
	}
	query.exec();
	
	while (query.next()) {
		amount = query.value(0).toString();
		section = query.value(1).toString();
		
		// National Liability QIF
		qif_national.append("\nD" + valuta);
		qif_national.append("\nT" + amount);
		qif_national.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString().arg(section));
		qif_national.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_national.append("\nM" + memo.arg(section));
		qif_national.append("\nL" + settings.value("contribution/account_donation", "Donation_%1").toString().arg(section));
		qif_national.append("\n^\n");
		
		// Section Income QIF
		QString qif_section("!Type:" + settings.value("qif/account_bank", "Bank").toString());
		qif_section.append("\nD" + valuta);
		qif_section.append("\nT" + amount);
		qif_section.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString().arg(section));
		qif_section.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_section.append("\nM" + memo.arg(section));
		qif_section.append("\nL" + settings.value("contribution/income_donation", "Donations").toString());
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
	
	// Mark all Invoices as Contributed - Don't use the Invoice-Class here in case of one vs. lot of queries
	query.prepare(QString("UPDATE donation_entities SET contributed=:now WHERE booked>=:start AND booked<=:end AND contributed<=:contr %1;").arg(notIn));
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":now", QDate::currentDate().toString("yyyy-MM-dd"));
	for (int i = 0; i < dontContribute.size(); i++) {
		query.bindValue(i+4, dontContribute.at(i));
	}
	query.exec();
}

void Donations::exportData() {
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

void Donations::sendEmail() {
	QSettings settings;
	Section sec;
	PPSPerson pers;
	QString subject, message, email;
	QString attachment = tr("Donations_%1.qif").arg(QDate::currentDate().toString("yyyy-MM-dd"));
	
	Smtp mail(settings.value("smtp/host", "localhost").toString(), settings.value("smtp/port", 587).toInt());
	if (settings.value("smtp/authentication", "login").toString() == "login") {
		mail.authLogin(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	} else {
		mail.authPlain(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	}
	
	// Send the Contributions by EMail
	createQif();
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
		subject = QString("Donation contributions for %1").arg(*section);
		message = "Ahoi Treasurer,\n"
		          "This time still in a not verry beautifull Mailing.\n"
		          "In future you will also receive an additional PDF-Evidence with all information included. This time, the evidence from your Bank has to be be enough.\n\n"
		          "Attatched you'l find the contribution of the donations of the Pirate Party Switzerland for your section as a Quicken-Interchange File. This you can easily import with GnuCash in your accounting through 'File' -> 'Import' -> 'Import QIF...'.\n"
		          "You will receive the settlement during next days, change the valuta after in the imported booking.\n\n"
		          "Greetings,\nLukas Zurschmiede\nTreasurer Pirate Party Switzerland\n";
		
		// Set the Mailtext and send the mail
		mail.setTextMessage(message);
		email = settings.value("smtp/from", "").toString();
		if (pers.email().length() > 0) {
			email = pers.email().at(0);
		}
		
		// If sending was not successfull (or no treasurer was found), let the user save the QIF
		if (email.isEmpty() || !mail.send(settings.value("smtp/from", "me@noreply.dom").toString(), email, subject)) {
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

void Donations::selectSection() {
	searchDonations();
	QSqlQuery query(db);
	query.prepare(QString("SELECT strftime('%Y',booked) AS year FROM donation_entities WHERE"
	                      " section%1:section AND contributed<=:contr GROUP BY year;").arg(
	                         (sectionList->currentIndex() == 0 || sectionList->count() == 0 ? "<>" : "=")
	                      ));
	
	query.bindValue(":section", sectionList->currentText());
	query.bindValue(":contr", QDate(1900, 1, 1).toString("yyyy-MM-dd"));
	query.exec();
	
	yearList->clear();
	while (query.next()) {
		yearList->addItem(query.value(0).toString());
	}
}

void Donations::selectYear() {
	searchDonations();
}

