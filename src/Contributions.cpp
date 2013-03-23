/*
	The Contribution-Widget - shows and let create Memberfee-Contributions
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

	connect(spinYear, SIGNAL(valueChanged(int)), this, SLOT(searchDataTimeout()));
	connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(dateUntil, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	connect(btnEmail, SIGNAL(clicked()), this, SLOT(sendEmail()));
	
	connect(sectionList, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSection()));
	connect(yearList, SIGNAL(itemSelectionChanged()), this, SLOT(selectYear()));
	
	tableModel = new QSqlQueryModel(tableView);
	contributionsModel = new QSqlQueryModel(contributionTable);
	
	loadData();
	loadSectionContributions();
}

Contributions::~Contributions() {}

void Contributions::loadData() {
	spinYear->setValue(QDate::currentDate().year());
	dateFrom->setDate(QDate::currentDate().addMonths(-4));
	dateUntil->setDate(QDate::currentDate());
	
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	
	tableModel->setHeaderData(0, Qt::Horizontal, tr("Paid Date"));
	tableModel->setHeaderData(1, Qt::Horizontal, tr("Requested"));
	tableModel->setHeaderData(2, Qt::Horizontal, tr("Paid"));
	tableModel->setHeaderData(3, Qt::Horizontal, tr("Person"));
	tableModel->setHeaderData(4, Qt::Horizontal, tr("City"));
	tableModel->setHeaderData(5, Qt::Horizontal, tr("Section"));
	tableModel->setHeaderData(6, Qt::Horizontal, tr("Member UID"));
	tableModel->setHeaderData(7, Qt::Horizontal, tr("Reference-Number"));

	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableView->setSortingEnabled(false);
	tableView->setModel(tableModel);
}

void Contributions::loadSectionContributions() {
	QSqlQuery query = createContributionsQuery();
	query.exec();
	contributionsModel->setQuery(query);
	
	contributionsModel->setHeaderData(0, Qt::Horizontal, tr("Paid Date"));
	contributionsModel->setHeaderData(1, Qt::Horizontal, tr("Requested"));
	contributionsModel->setHeaderData(2, Qt::Horizontal, tr("Paid"));
	contributionsModel->setHeaderData(3, Qt::Horizontal, tr("Person"));
	contributionsModel->setHeaderData(4, Qt::Horizontal, tr("City"));
	contributionsModel->setHeaderData(5, Qt::Horizontal, tr("Section"));
	contributionsModel->setHeaderData(6, Qt::Horizontal, tr("Member UID"));
	contributionsModel->setHeaderData(7, Qt::Horizontal, tr("Reference-Number"));

	contributionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	contributionTable->setSortingEnabled(false);
	contributionTable->setModel(contributionsModel);
	
	QSqlQuery sections(db);
	sections.prepare("SELECT for_section FROM pps_invoice GROUP BY for_section ORDER BY for_section ASC;");
	sections.exec();
	sectionList->addItem(tr("all"));
	while (sections.next()) {
		sectionList->addItem(sections.value(0).toString());
	}
}

QSqlQuery Contributions::createQuery() {
	QSqlQuery query(db);
	query.prepare("SELECT paid_date,amount,amount_paid,address_name,address_city,for_section,member_uid,reference FROM pps_invoice WHERE"
	              " paid_date>=:start AND paid_date<=:end AND payable_date>=:year_begin AND payable_date<=:year_end AND state=:state"
	              " ORDER BY for_section,paid_date ASC;");
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":state", Invoice::StatePaid);
	// only contribute payments til the end of the year
	query.bindValue(":year_begin", QDate(spinYear->value(), 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":year_end", QDate(spinYear->value(), 12, 31).toString("yyyy-MM-dd"));
	return query;
}

QSqlQuery Contributions::createContributionsQuery() {
	QDate from, to;
	QSqlQuery query(db);
	
	if (yearList->selectedItems().count() > 0 && yearList->currentRow() > 0) {
		from.setDate(yearList->selectedItems().at(0)->text().toInt(), 01, 01);
		to.setDate(yearList->selectedItems().at(0)->text().toInt(), 12, 31);
	} else {
		from.setDate(1900, 01, 01);
		to = QDate::currentDate();
	}
	
	query.prepare(QString("SELECT paid_date,amount,amount_paid,address_name,address_city,for_section,member_uid,reference FROM pps_invoice WHERE"
	                      " for_section%1:section AND paid_date>=:start"
	                      " AND paid_date<=:end AND state=:state ORDER BY for_section ASC;").arg(
	                         (sectionList->currentIndex() == 0 || sectionList->count() == 0 ? "<>" : "=")
	                      ));
	query.bindValue(":start", from.toString("yyyy-MM-dd"));
	query.bindValue(":end", to.toString("yyyy-MM-dd"));
	query.bindValue(":state", Invoice::StateContributed);
	query.bindValue(":section", sectionList->currentText());
	return query;
}

void Contributions::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	searchData();
}

void Contributions::searchDataTimeout() {
	killTimer(searchTimer);
	searchTimer = startTimer(1000);
}

void Contributions::searchData() {
	QSqlQuery query = createQuery();
	query.exec();
	tableModel->setQuery(query);
	showOverview();
}

void Contributions::searchContributions() {
	QSqlQuery query = createContributionsQuery();
	query.exec();
	contributionsModel->setQuery(query);
	showContributionsDetails();
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
	QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
	dontContribute.removeAll("");
	
	// Request all payments in the selected daterange
	query.prepare("SELECT"
#ifndef FIO
		" SUM(amount/2) as amount,for_section"
#else
		" amount,amount_paid,recommendations"
#endif
		" FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end"
		" AND payable_date>=:year_begin AND payable_date<=:year_end AND state=:state"
#ifndef FIO
		" GROUP BY for_section"
#endif
		" ORDER BY for_section ASC;");
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":state", Invoice::StatePaid);
	// only contribute payments till the end of the selected year
	query.bindValue(":year_begin", QDate(spinYear->value(), 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":year_end", QDate(spinYear->value(), 12, 31).toString("yyyy-MM-dd"));
	query.exec();
	
	double sum = 0.0;
	int row = 0;
	
#ifndef FIO
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
#endif
//#else
	// First calculate all recommendated values from all payments
	FIOCalc *fio = new FIOCalc;
	QList< contribution_data * > cdata;
	contribution_data *data;
	while (query.next()) {
		QStringList tmp;
		QStringList recom = query.value(2).toString().split(";");

		// Calculate the sections parts - do not regard invalid sections
		for (int i = 0; i < recom.size(); i++) {
			tmp = recom.at(i).split(":");
			if (tmp.size() == 2) {
				fio->addSection( tmp.at(0), tmp.at(1).toFloat() );
			}
		}
		QHash<QString, float> result = fio->calculate(query.value(1).toFloat());

		// Go through all section from this payment and register the amounts.
		QHash<QString, float>::const_iterator it = result.constBegin();
		while (it != result.constEnd()) {
			data = NULL;
			for (int i = 0; i < cdata.size(); i++) {
				if (cdata.at(i)->section == it.key()) {
					data = cdata.at(i);
					break;
				}
			}

			if (!data) {
				data = new contribution_data;
				data->section = it.key();
				data->sum = 0;
				cdata.append(data);
			}
			data->amount_list.append(it.value());
			data->sum += it.value();

			it++;
		}
	}

	// Fill the details with the calculated recommend values
	while (!cdata.isEmpty()) {
		contribution_data *cd = cdata.takeFirst();
		QLabel *s = new QLabel( tr("Section: %1").arg( cd->section ) );
		QLabel *a = new QLabel( tr("%1 sFr.").arg( cd->sum, 0, 'f', 2) );
		a->setAlignment(Qt::AlignRight);
		if (dontContribute.contains( cd->section )) {
			s->setStyleSheet("QLabel { color: darkRed }");
			a->setStyleSheet("QLabel { color: darkRed }");
		} else {
			sum += cd->sum;
		}
		overviewLayout->addWidget(s, row, 0);
		overviewLayout->addWidget(a, row, 1);
		row++;
		delete cd;
	}
//#endif
	
	// Total
	QLabel *s = new QLabel( tr("<b>Total:</b>") );
	QLabel *a = new QLabel( tr("<b>%1 sFr.</b>").arg(sum, 0, 'f', 2) );
	a->setAlignment(Qt::AlignRight);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(a, row, 1);
}

void Contributions::showContributionsDetails() {
	QSettings settings;
	QSqlQuery query = createContributionsQuery();
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

void Contributions::createQif() {
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
		notIn.remove(0, 1).append(")").prepend("AND for_section NOT IN (");
	}
	
	// Initialize the QIF and the list with all Section-QIFs
	sectionQif.clear();
	QString qif_national("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	if (!memoText->text().isEmpty()) {
		memo = memoText->text();
	}
	
	// Get all Data
	query.prepare(QString("SELECT SUM(amount/2) AS amount,for_section FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end"
	                      " AND state=:state AND payable_date<=:year %1 GROUP BY for_section;").arg(notIn));
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":state", Invoice::StatePaid);
	// only contribute payments til the end of the year - those have to be in the balance of the next year
	query.bindValue(":year", QDate(dateUntil->date().year(), 12, 31).toString("yyyy-MM-dd"));
	
	for (int i = 0; i < dontContribute.size(); i++) {
		query.bindValue(i+4, dontContribute.at(i));
	}
	query.exec();
	if (query.lastError().type() != QSqlError::NoError) {
		qDebug() << query.lastQuery();
		qDebug() << query.lastError();
	}
	while (query.next()) {
		amount = query.value(0).toString();
		section = query.value(1).toString();
		
		// National Liability QIF
		qif_national.append("\nD" + valuta);
		qif_national.append("\nT" + amount);
		qif_national.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString().arg(section));
		qif_national.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_national.append("\nM" + memo.arg(section));
		qif_national.append("\nL" + settings.value("contribution/account", "Contribution_%1").toString().arg(section));
		qif_national.append("\n^\n");
		
		// Section Income QIF
		QString qif_section("!Type:" + settings.value("qif/account_bank", "Bank").toString());
		qif_section.append("\nD" + valuta);
		qif_section.append("\nT" + amount);
		qif_section.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString().arg(section));
		qif_section.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_section.append("\nM" + memo.arg(section));
		qif_section.append("\nL" + settings.value("contribution/income", "Membership fee").toString());
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
	query.prepare(QString("UPDATE pps_invoice SET state=:newstate WHERE paid_date>=:start AND paid_date<=:end AND payable_date<=:year AND state=:state %1;").arg(notIn));
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":state", Invoice::StatePaid);
	query.bindValue(":newstate", Invoice::StateContributed);
	// only contribute payments til the end of the year - those have to be in the balance of the next year
	query.bindValue(":year", QDate(dateUntil->date().year(), 12, 31).toString("yyyy-MM-dd"));
	
	for (int i = 0; i < dontContribute.size(); i++) {
		query.bindValue(i+5, dontContribute.at(i));
	}
	query.exec();
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
	QString attachment = tr("Contribution_%1.qif").arg(QDate::currentDate().toString("yyyy-MM-dd"));
	
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
		subject = QString("Membership fee contributions for %1").arg(*section);
		message = "Ahoi Treasurer,\n"
		          "This time still in a not verry beautifull Mailing.\n"
		          "In future you will also receive an additional PDF-Evidence with all information included. This time, the evidence from your Bank has to be be enough.\n\n"
		          "Attatched you'l find the contribution of the membership fee of the Pirate Party Switzerland for your section as a Quicken-Interchange File. This you can easily import with GnuCash in your accounting through 'File' -> 'Import' -> 'Import QIF...'.\n"
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

void Contributions::selectSection() {
	searchContributions();
	QSqlQuery query(db);
	query.prepare(QString("SELECT strftime('%Y',paid_date) AS year FROM pps_invoice WHERE"
	                      " for_section%1:section AND state=:state GROUP BY year;").arg(
	                         (sectionList->currentIndex() == 0 || sectionList->count() == 0 ? "<>" : "=")
	                      ));
	
	query.bindValue(":state", Invoice::StateContributed);
	query.bindValue(":section", sectionList->currentText());
	query.exec();
	
	yearList->clear();
	while (query.next()) {
		yearList->addItem(query.value(0).toString());
	}
}

void Contributions::selectYear() {
	searchContributions();
}



