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

Contributions::Contributions(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	QDate now = QDate::currentDate();
	recalcYear->setValue(now.year());
	recalcQ1->setDate(QDate(now.year(), 4, 1));
	recalcQ2->setDate(QDate(now.year(), 7, 1));
	recalcQ3->setDate(QDate(now.year(), 10, 1));
	recalcQ4->setDate(QDate(now.year() + 1, 1, 1));
	recalcPeriodFrom->setDate(QDate(now.year() - 1, 12, 21));
	recalcPeriodUntil->setDate(QDate(now.year(), 12, 31));
	
#ifndef FIO
	checkNoFIO->setChecked(TRUE);
	checkNoFIO->hide();
#endif

	connect(spinYear, SIGNAL(valueChanged(int)), this, SLOT(searchDataTimeout()));
	connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(dateUntil, SIGNAL(dateChanged(QDate)), this, SLOT(searchDataTimeout()));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportData()));
	connect(btnEmail, SIGNAL(clicked()), this, SLOT(sendEmail()));
	connect(sectionList, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSection()));
	connect(yearList, SIGNAL(itemSelectionChanged()), this, SLOT(selectYear()));
	connect(dateList, SIGNAL(itemSelectionChanged()), this, SLOT(selectDate()));
	connect(btnSendDetail, SIGNAL(clicked()), this, SLOT(sendContributionDetails()));
	connect(btnExportDetail, SIGNAL(clicked()), this, SLOT(exportContributionDetails()));
	connect(recalcButton, SIGNAL(clicked()), this, SLOT(recalculateContributions()));

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

	QSqlQuery query = createTableModelQuery();
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
	contributionsModel->setHeaderData(1, Qt::Horizontal, tr("Contributed"));
	contributionsModel->setHeaderData(2, Qt::Horizontal, tr("Person"));
	contributionsModel->setHeaderData(3, Qt::Horizontal, tr("City"));
	contributionsModel->setHeaderData(4, Qt::Horizontal, tr("Section"));
	contributionsModel->setHeaderData(5, Qt::Horizontal, tr("Member UID"));
	contributionsModel->setHeaderData(6, Qt::Horizontal, tr("Reference-Number"));
	contributionsModel->setHeaderData(7, Qt::Horizontal, tr("Year"));
	contributionsModel->setHeaderData(8, Qt::Horizontal, tr("Contributed"));

	contributionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	contributionTable->setSortingEnabled(false);
	contributionTable->setModel(contributionsModel);

	QList<QString> sections;
	Section::getSectionList(&sections);
	sectionList->addItem(tr("all"));
	recalcSection->addItem(tr("all"));
	for (int i = 0; i < sections.size(); i++) {
		sectionList->addItem(sections.at(i));
		recalcSection->addItem(sections.at(i));
	}
}

QSqlQuery Contributions::createTableModelQuery() {
	QSqlQuery query(db);
	query.prepare("SELECT paid_date,amount,amount_paid,address_name,address_city,for_section,member_uid,reference FROM pps_invoice WHERE"
		" paid_date>=:start AND paid_date<=:end AND payable_date>=:year_begin AND payable_date<=:year_end AND state=:state"
		" ORDER BY for_section,paid_date ASC;");
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":state", (int)Invoice::StatePaid);
	// only contribute payments til the end of the year
	query.bindValue(":year_begin", QDate(spinYear->value(), 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":year_end", QDate(spinYear->value(), 12, 31).toString("yyyy-MM-dd"));
	return query;
}

QSqlQuery Contributions::createContributionsQuery() {
	qint32 year;
	QString selection_date;
	QSqlQuery query(db);

	if (yearList->selectedItems().count() > 0 && yearList->currentRow() >= 0) {
		year = yearList->selectedItems().at(0)->text().toInt();
	} else {
		year = QDate::currentDate().year();
	}

	if (dateList->selectedItems().count() > 0 && dateList->currentRow() > 0) {
		selection_date = dateList->selectedItems().at(0)->data(Qt::UserRole).toDate().toString("yyyy-MM-dd");
	}

	query.prepare(QString("SELECT inv.paid_date,cont.amount,inv.address_name,inv.address_city,cont.section,"
		"inv.member_uid,cont.reference,cont.year,cont.contribute_date FROM pps_invoice AS inv, pps_contribution cont"
		" WHERE inv.reference = cont.reference AND cont.section %1 :section"
		" AND cont.year=:year AND cont.state=:state %2"
		" ORDER BY cont.section,inv.address_name ASC;").arg(
			(sectionList->currentIndex() == 0 || sectionList->count() == 0 ? "<>" : "="),
			(selection_date.isEmpty() ? "" : " AND cont.contribute_date = :date")
		));
	query.bindValue(":section", sectionList->currentText());
	query.bindValue(":year", year);
	query.bindValue(":state", (int)Invoice::StateContributed);
	if (!selection_date.isEmpty()) {
		query.bindValue(":date", selection_date);
	}
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
#ifdef FIO
	while (!l_contrib_data.isEmpty()) {
		delete l_contrib_data.takeFirst();
	}
#endif
	QSqlQuery query = createTableModelQuery();
	query.exec();
	tableModel->setQuery(query);
	showOverview();
}

void Contributions::searchContributions() {
	QSqlQuery query = createContributionsQuery();
	query.exec();
	contributionsModel->setQuery(query);
	showBookedContributions();
}

void Contributions::showOverview() {
	QSettings settings;
	QSqlQuery query = contributionsQuery(dateFrom->date(), dateUntil->date(), spinYear->value(), "", Invoice::StatePaid);

	// Clean the GridContainer
	QLayoutItem *child;
	while ((child = overviewLayout->takeAt(0)) != 0) {
		delete ((QLabel *)child->widget());
		delete child;
	}

	// Load "do not contribute this sections" list
	QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
	dontContribute.removeAll("");

	// Get all payments in the selected daterange
	double sum = 0.0, held_back = 0.0;
	int row = 0, num_total = 0, num_paid = 0, num_held = 0;
	QString section, num_members;
	float amount;

#ifndef FIO
	while (query.next()) {
		section = query.value(1).toString();
		amount = query.value(0).toFloat() / 2;
		num_members = query.value(2).toString();
		num_total += query.value(2).toInt();
#else
	QList< contribution_data * > cdata;
	calculateFioContribution(&cdata, &query, 1, 2, 3, 4);
	num_total = query.size();
	if (num_total < 0) {
		num_total = 0;
		query.seek(-1);
		while (query.next()) {
			num_total++;
		}
	}

	while (!cdata.isEmpty()) {
		contribution_data *cd = cdata.takeFirst();
		section = cd->section;
		amount = cd->sum;
		num_members = QString::number(cd->amount_list.size());
#endif
		QLabel *s = new QLabel( tr("<b>%1</b>").arg(Section::getSectionDescription(section)) );
		QLabel *a = new QLabel( tr("%1 sFr.").arg(amount, 0, 'f', 2) );
		QLabel *n = new QLabel( tr("%1 members").arg(num_members) );
		a->setAlignment(Qt::AlignRight);
		n->setAlignment(Qt::AlignRight);
		s->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		a->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		n->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

		if ( dontContribute.contains(section) ) {
			s->setStyleSheet("QLabel { color: darkRed }");
			a->setStyleSheet("QLabel { color: darkRed }");
			n->setStyleSheet("QLabel { color: darkRed }");
			held_back += amount;
#ifdef FIO
			num_held += cd->amount_list.size();
#else
			num_held += query.value(2).toInt();
#endif
		} else {
			sum += amount;
#ifdef FIO
			num_paid += cd->amount_list.size();
#else
			num_paid += query.value(2).toInt();
#endif
		}

		overviewLayout->addWidget(s, row, 0);
		overviewLayout->addWidget(n, row, 1);
		overviewLayout->addWidget(a, row, 2);
		row++;
#ifdef FIO
		delete cd;
#endif
	}

	// Add a blank line
	overviewLayout->addWidget(new QLabel(""), row, 0);
	row++;

	// Totals
	QLabel *s = new QLabel( tr("<b>Total pay out:</b>") );
	QLabel *a = new QLabel( tr("<b>%1 sFr.</b>").arg(sum, 0, 'f', 2) );
	QLabel *n = new QLabel( tr("<b>%1 dues</b>").arg(num_paid) );
	a->setAlignment(Qt::AlignRight);
	n->setAlignment(Qt::AlignRight);
	s->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	a->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	n->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(n, row, 1);
	overviewLayout->addWidget(a, row, 2);
	row++;

	s = new QLabel( tr("<b>Held back:</b>") );
	a = new QLabel( tr("<b>%1 sFr.</b>").arg(held_back, 0, 'f', 2) );
	n = new QLabel( tr("<b>%1 dues</b>").arg(num_held) );
	a->setAlignment(Qt::AlignRight);
	n->setAlignment(Qt::AlignRight);
	s->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	a->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	n->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(n, row, 1);
	overviewLayout->addWidget(a, row, 2);
	row++;

	// Add a blank line
	overviewLayout->addWidget(new QLabel(""), row, 0);
	row++;

	s = new QLabel( tr("<b>Total contributions:</b>") );
	a = new QLabel( tr("<b>%1 sFr.</b>").arg(held_back + sum, 0, 'f', 2) );
	n = new QLabel( tr("<b>%1 payments</b>").arg(num_total) );
	a->setAlignment(Qt::AlignRight);
	n->setAlignment(Qt::AlignRight);
	s->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	a->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	n->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	overviewLayout->addWidget(s, row, 0);
	overviewLayout->addWidget(n, row, 1);
	overviewLayout->addWidget(a, row, 2);
}

#ifdef FIO
void Contributions::calculateFioContribution( QList< contribution_data * > *cdata, QSqlQuery *query, int col_amount, int col_recom, int col_reference, int col_valuta) {
	contribution_data *data, *odata;

	// we cache this to not calculate this each time
	if (!l_contrib_data.isEmpty()) {
		contribution_data *orig;
		for (int i = 0; i < l_contrib_data.size(); i++) {
			orig = l_contrib_data.at(i);
			data = new contribution_data;
			data->section = QString(orig->section);
			data->sum = float(orig->sum);
			data->amount_list = QList< contribution_data_invoice >(orig->amount_list);
			cdata->append(data);
		}
		return;
	}

	QString name, reference;
	FIOCalc *fio = new FIOCalc;
	query->seek(-1);
	while (query->next()) {
		QStringList tmp, recom = query->value(col_recom).toString().split(";");
		reference = query->value(col_reference).toString();

		// Calculate the sections parts - do not regard invalid sections
		fio->reset();
		for (int i = 0; i < recom.size(); i++) {
			tmp = recom.at(i).split(":");
			if (tmp.size() == 2) {
				name = Section::getSectionName(tmp.at(0));
				name = name.isEmpty() ? tmp.at(0) : name;
				fio->addSection( name, tmp.at(1).toFloat() );
			}
		}
		QHash<QString, float> result = fio->calculate(query->value(col_amount).toFloat());

		// Go through all section from this payment and register the amounts.
		QHash<QString, float>::const_iterator it = result.constBegin();
		while (it != result.constEnd()) {
			data = NULL;
			odata = NULL;
			for (int i = 0; i < cdata->size(); i++) {
				if (cdata->at(i)->section == it.key()) {
					data = cdata->at(i);
					odata = l_contrib_data.at(i);
					break;
				}
			}

			if (!data) {
				data = new contribution_data;
				odata = new contribution_data;
				data->section = it.key();
				odata->section = it.key();
				data->sum = 0;
				odata->sum = 0;
				cdata->append(data);
				l_contrib_data.append(odata);
			}
			contribution_data_invoice d1 = { reference, it.value(), query->value(col_valuta).toDate() };
			contribution_data_invoice d2 = { reference, it.value(), query->value(col_valuta).toDate() };
			data->amount_list.append( d1 );
			odata->amount_list.append( d2 );
			data->sum += it.value();
			odata->sum += it.value();

			it++;
		}
	}
	delete fio;
}
#endif

void Contributions::showBookedContributions() {
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
	QMap<QString, double> sum;
	QMap<QString, int> num;
	QString section;
	while (query.next()) {
		section = query.value(4).toString();
		if (!sum.contains(section)) {
			sum.insert(section, 0);
			num.insert(section, 0);
		}
		num.insert(section, num.value(section) + 1);
		sum.insert(section, sum.value(section) + query.value(1).toDouble());
	}

	int row = 0;
	int ctotal = 0;
	double stotal = 0.0;
	QMap<QString, double>::const_iterator iter = sum.constBegin();
	while (iter != sum.constEnd()) {
		stotal += iter.value();
		ctotal += num.find(iter.key()).value();

		QLabel *t = new QLabel( tr("<b>%1</b>").arg( Section::getSectionDescription(iter.key()) ) );
		t->setAlignment(Qt::AlignLeft);
		t->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		detailsLayout->addWidget(t, row, 0);

		QLabel *p = new QLabel( tr("%1 payments").arg( num.find(iter.key()).value() ) );
		p->setAlignment(Qt::AlignRight);
		p->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		detailsLayout->addWidget(p, row, 1);

		QLabel *v = new QLabel( QString("%1 sFr.").arg(iter.value(), 0, 'f', 2) );
		v->setAlignment(Qt::AlignRight);
		v->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		detailsLayout->addWidget(v, row, 2);
		row++;
		iter++;
	}

	// Totals
	QLabel *kp = new QLabel( tr("<b>Total:</b>") );
	QLabel *vp = new QLabel( QString("<b>%1 payments</b>").arg(ctotal) );
	QLabel *vt = new QLabel( QString("<b>%1 sFr.</b>").arg(stotal, 0, 'f', 2) );
	kp->setAlignment(Qt::AlignLeft);
	vp->setAlignment(Qt::AlignRight);
	vt->setAlignment(Qt::AlignRight);
	kp->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vp->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vt->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	detailsLayout->addWidget(kp, row, 0);
	detailsLayout->addWidget(vp, row, 1);
	detailsLayout->addWidget(vt, row, 2);
}

void Contributions::createQif() {
	QSettings settings;
	QString date = QDate::currentDate().toString("yyyyMMdd");
	QString memo = settings.value("contribution/memo", "Contribution: %1").toString();
	QString section, amount;
	QSqlQuery query2(db);
	QDate valuta = QDate::currentDate();
	int year = spinYear->value();
	qint32 num_contrib = 0, progress_contrib = 0;
	
	// Initialize the QIF and the list with all Section-QIFs
	sectionQif.clear();
	QString qif_national("!Type:" + settings.value("qif/account_liability", "Oth L").toString());
	if (!memoText->text().isEmpty()) {
		memo = memoText->text();
	}
	
	// Load "do not contribute this sections" list
	QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
	dontContribute.removeAll("");
#ifndef FIO
	QString notIn(QString("?,").repeated( dontContribute.size() ));
	notIn.remove(notIn.length()-1, 1); // remove last ','
#endif
	
	// Prepare query2 for insert all contribution in a separate table
	query2.prepare("INSERT INTO pps_contribution (reference,state,section,amount,contribute_date,year) VALUES (:reference,:state,:section,:amount,:date,:year);");
	
	// Show a Progressbar
	QProgressDialog progress(tr("Preparing and save contribution..."), tr("Please wait..."), 0, 1, this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	cancelProgress = FALSE;
	connect(&progress, SIGNAL(canceled()), this, SLOT(cancelRecreate()));
	
	// Create the query with all new contributions.
	QSqlQuery query = contributionsQuery(dateFrom->date(), dateUntil->date(), spinYear->value(), section, Invoice::StatePaid);
	
#ifndef FIO
	QHash<QString, float> contributions;
	float famount;
	while (query.next()) {
		// TODO: No grouping by the section any longer in the query, do this here.
		num_contrib++;
		famount = query.value(0).toFloat() / 2;
		section = query.value(1).toString();
		contributions.insert(section, contributions.value(section, 0) + famount);
	}
	progress.setMaximum(num_contrib);
	
	QHash<QString, float>::const_iterator it = contributions.constBegin();
	while (it != contributions.constEnd()) {
		amount = QString::number(it.value());
		section = it.key();
#else
	QList< contribution_data * > cdata;
	calculateFioContribution(&cdata, &query, 1, 2, 3, 4);

	// For the status bar: Get the number of saving contributions
	for (int i = 0; i < cdata.size(); i++) {
		num_contrib += cdata.at(i)->amount_list.size();
	}
	progress.setMaximum(num_contrib);

	// Fill the details with the calculated recommend values
	for (int i = 0; i < cdata.size(); i++) {
		contribution_data *cd = cdata.at(i);
		
		// If the process is cancelled, we need to clean up the data but not process them
		if (cancelProgress || dontContribute.contains(cd->section)) {
			delete cd;
			continue;
		}
		section = cd->section;
		
		// Save all contribution parts into a separate table
		query2.bindValue(":section", cd->section);
		query2.bindValue(":state", Invoice::StateContributed);
		query2.bindValue(":year", year);
		for (int i = 0; i < cd->amount_list.size(); i++) {
			progress.setValue(++progress_contrib);
			
			query2.bindValue(":reference", cd->amount_list.at(i).reference);
			query2.bindValue(":amount", QString::number(cd->amount_list.at(i).amount));
			query2.bindValue(":date", valuta);
			query2.exec();
		}
		
		amount = QString::number(cd->sum);
		delete cd;
#endif

		// National Liability QIF
		qif_national.append("\nD" + valuta.toString("yyyy-MM-dd"));
		qif_national.append("\nT-" + amount);
		qif_national.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString());
		qif_national.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_national.append("\nM" + memo.arg(section));
		qif_national.append("\nL" + settings.value("contribution/account", "Contribution %1").toString().arg(section));
		qif_national.append("\n^\n");

		// Section Income QIF
		QString qif_section("!Type:" + settings.value("qif/account_bank", "Bank").toString());
		qif_section.append("\nD" + valuta.toString("yyyy-MM-dd"));
		qif_section.append("\nT" + amount);
		qif_section.append("\nP"+ settings.value("contribution/payer", "Pirateparty Switzerland").toString());
		qif_section.append("\nNCONT-" + date + "-" + section.toUpper());
		qif_section.append("\nM" + memo.arg(section));
		qif_section.append("\nL" + settings.value("contribution/income", "Membership fee %1").toString().arg(section));
		qif_section.append("\n^\n");

		sectionQif.insert(section, qif_section);
	}
	progress.setValue(num_contrib);

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
	query.prepare(QString("UPDATE pps_invoice SET state=:newstate WHERE paid_date>=:start AND paid_date<=:end"
		" AND payable_date>=:year_begin AND payable_date<=:year_end AND state=:state"
#ifndef FIO
		" AND section NOT IN (%1);").arg(notIn)
#else
		";")
#endif
	);
	query.bindValue(":newstate", (int)Invoice::StateContributed);
	query.bindValue(":start", dateFrom->date().toString("yyyy-MM-dd"));
	query.bindValue(":end", dateUntil->date().toString("yyyy-MM-dd"));
	query.bindValue(":year_begin", QDate(spinYear->value(), 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":year_end", QDate(spinYear->value(), 12, 31).toString("yyyy-MM-dd"));
	query.bindValue(":state", (int)Invoice::StatePaid);
#ifndef FIO
	for (int i = 0; i < dontContribute.size(); i++) {
		query.bindValue(i+6, dontContribute.at(i));
	}
#endif
	if (!query.exec()) {
		qDebug() << query.lastError();
	}
}

QSqlQuery Contributions::contributionsQuery(QDate from, QDate until, int payable_year, QString section, Invoice::State state, bool noFioCalculation) {
	QSettings settings;
	QSqlQuery query(db);
	QStringList dontContribute;
	QString notIn;
	
	// Load "do not contribute this sections" list
	if (noFioCalculation) {
		dontContribute.append(settings.value("contribution/dontpay", "members").toStringList());
		dontContribute.removeAll("");
		notIn = QString("?,").repeated(dontContribute.size());
		notIn.remove(notIn.length()-1, 1); // remove last ','
	}
	
	// Get all payments in the selected daterange
	QString sql = "SELECT"
	" amount,amount_paid,recommendations,reference,paid_date,for_section"
	" FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end"
	" AND payable_date>=:year_begin AND payable_date<=:year_end AND state=:state";
	if (!section.isEmpty()) {
		sql.append(" AND for_section=:section");
	}
	if (noFioCalculation) {
		sql.append(QString(" AND for_section NOT IN (%1) ORDER BY for_section ASC;").arg(notIn));
	} else {
		sql.append(" ORDER BY for_section ASC;");
	}
	
	query.prepare(sql);
	query.bindValue(":start", from.toString("yyyy-MM-dd"));
	query.bindValue(":end", until.toString("yyyy-MM-dd"));
	query.bindValue(":state", (int)state);
	if (!section.isEmpty()) {
		query.bindValue(":section", section);
	}
	
	// Only contribute payments in the requested year
	query.bindValue(":year_begin", QDate(payable_year, 1, 1).toString("yyyy-MM-dd"));
	query.bindValue(":year_end", QDate(payable_year, 12, 31).toString("yyyy-MM-dd"));
	
	if (noFioCalculation) {
		int offset = section.isEmpty() ? 5 : 6;
		for (int i = 0; i < dontContribute.size(); i++) {
			query.bindValue(i + offset, dontContribute.at(i));
		}
	}
	query.exec();
	return query;
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
	QString subject, message, email, fileName, section_file;
	QString attachment = tr("Contribution_%1.qif").arg(QDate::currentDate().toString("yyyy-MM-dd"));
	QString attachment_pdf = tr("Contribution_%1.pdf").arg(QDate::currentDate().toString("yyyy-MM-dd"));

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
		mail.clearAttachments();
		sec.load(*section);
		pers.load(sec.treasurer());

		// Create and attach the statistical PDF for selected time span
		XmlPdf *pdf = getPdf(*section, QDate(spinYear->value(), 1, 1), QDate(spinYear->value(), 12, 31));
		pdf->setVar("section_name", sec.name());
		pdf->setVar("section_treasurer", pers.givenName() + " " + pers.familyName());
		pdf->setVar("section_address", sec.address());
		pdf->setVar("section_fullname", sec.name());
		pdf->setVar("section_email", pers.email().first());
		pdf->setVar("pp_country", settings.value("pdf/var_pp_country", "CH").toString());
		pdf->setVar("pp_zip", settings.value("pdf/var_pp_zip", "1337").toString());
		pdf->setVar("pp_city", settings.value("pdf/var_pp_city", "Vallorbe").toString());

		char fname1 [L_tmpnam];
		tmpnam(fname1);
		QString fileName(fname1);
		pdf->print(fileName);
		mail.attach(fileName, attachment_pdf);

		// Create a tmp file form the QIF
		char fname2 [L_tmpnam];
		tmpnam(fname2);
		QString tmpFileName(fname2);
		QFile f(tmpFileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			out << sectionQif.value(*section);
			out.flush();
			f.close();
		}
		mail.attach(tmpFileName, attachment);

		// TODO: Load Subject and Texts
		subject = QString("Membership fee contributions for %1").arg(*section);
		message = "Ahoi Treasurer,\n"
			"Attatched you'l find the contribution of the membership fee of the Pirate Party Switzerland for your section as a Quicken-Interchange File as well as a pdf with an anonymized list of all payments. The QIF you can easily import in GnuCash or any othe accounting software through 'File' -> 'Import' -> 'Import QIF...'.\n"
			"You will receive the settlement during next days, so you need to change the valuta from the imported booking after.\n\n"
			"Greetings,\nNational Treasurer Pirate Party Switzerland\n";

		// Set the Mailtext and send the mail
		mail.setTextMessage(message);
		email = settings.value("smtp/from", "").toString();
		if (pers.email().length() > 0) {
			email = pers.email().at(0);
		}

		// If sending was not successfull (or no treasurer was found), let the user save the QIF
		if (email.isEmpty() || !mail.send(settings.value("smtp/from", "me@noreply.dom").toString(), email, subject)) {
			if (fileName.isEmpty()) {
				fileName = QFileDialog::getSaveFileName(this, tr("Save Contribution for the section"), "", tr("Quicken Interchange Format (*.qif)"));
			}
			section_file = tmpFileName;
			section_file.replace(".qif", "_" + (*section) + ".qif");
			if (!fileName.isEmpty()) {
				if (QFile::exists( section_file )) {
					QFile::remove( section_file );
				}
				if (!QFile::copy(fname2, section_file)) {
					qDebug() << "Error while copying the QIF: " << fname2 << " to " << section_file;
				}
			} else {
				qDebug() << "Error, no file name given to copy the QIF to...";
			}
		}

		// Remove temporary files and clean up
		remove(fname1);
		remove(fname2);
		delete pdf;
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
	dateList->clearSelection();

	if (yearList->selectedItems().count() > 0 && yearList->currentRow() >= 0) {
		fillContributionDateList(yearList->selectedItems().at(0)->text().toInt());
	} else {
		fillContributionDateList(QDate::currentDate().year());
	}

	searchContributions();
}

void Contributions::fillContributionDateList(int year) {
	QSqlQuery query(db);

	query.prepare("SELECT contribute_date FROM pps_contribution WHERE year=:year GROUP BY contribute_date;");
	query.bindValue(":year", year);
	query.exec();

	dateList->clear();
	dateList->addItem("all");
	while (query.next()) {
		QListWidgetItem *item = new QListWidgetItem;
		item->setData(Qt::UserRole, query.value(0).toDate());
		item->setText(query.value(0).toDate().toString("d. MMMM"));
		dateList->addItem(item);
	}
}

void Contributions::selectDate() {
	searchContributions();
}

void Contributions::exportContributionDetails() {
	XmlPdf *pdf = createContributionsPdf("Members", TRUE);
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Contributions overview as PDF"), "", tr("Portable Document Format (*.pdf)"));
	if (!fileName.isEmpty()) {
		pdf->print(fileName);
	}
	delete pdf;
}

void Contributions::sendContributionDetails() {

}

XmlPdf *Contributions::createContributionsPdf(QString section, bool showAll) {
	QSettings settings;
	QString date_format = settings.value("pdf/date_format", "dd.MM.yyyy").toString();
	QString year;
	XmlPdf *pdf = new XmlPdf;
	XmlPdfEntry *entry;
	QSqlQuery query(db);
	qint32 num = 0;
	float total = 0;

	pdf->loadTemplate(settings.value("pdf/contribution_template", "data/contribution.xml").toString());

	for (int i = 0; i < contributionsModel->rowCount(); i++) {
		if (!showAll && (section != contributionsModel->data(contributionsModel->index(i, 4)).toString())) {
			continue;
		}
		entry = pdf->addEntry("payment");
		entry->setVar("num", ++num);
		entry->setVar("section", contributionsModel->data(contributionsModel->index(i, 4)).toString());
		entry->setVar("paid", contributionsModel->data(contributionsModel->index(i, 0)).toDate().toString(date_format));
		entry->setVar("amount", contributionsModel->data(contributionsModel->index(i, 1)).toFloat());
		entry->setVar("city", contributionsModel->data(contributionsModel->index(i, 3)).toString());
		entry->setVar("date", contributionsModel->data(contributionsModel->index(i, 8)).toDate().toString(date_format));
		entry->setVar("year", contributionsModel->data(contributionsModel->index(i, 7)).toInt());
		entry->setVar("member", contributionsModel->data(contributionsModel->index(i, 5)).toInt());

		total += query.value(2).toFloat();
		if (i == 0) {
			year = contributionsModel->data(contributionsModel->index(i, 7)).toString();
		}
	}
	pdf->setVar("section_name", section);
	pdf->setVar("contribution_year", QDate::currentDate().toString("yyyy"));
	pdf->setVar("contribution_number", QString::number(num));
	pdf->setVar("contribution_total", QString::number(total, 'f', 2));
	pdf->setVar("contribution_average", QString::number(total/num, 'f', 2));
	pdf->setVar("print_city", settings.value("pdf/var_print_city", "Vallorbe").toString());
	pdf->setVar("print_date", QDate::currentDate().toString(date_format));

	return pdf;
}

XmlPdf *Contributions::getPdf(const QString &section, const QDate &from, const QDate &to) const {
	QSettings settings;
	QString date_format = settings.value("pdf/date_format", "dd.MM.yyyy").toString();
	XmlPdf *pdf = new XmlPdf;
	XmlPdfEntry *entry;
	QSqlQuery query(db);
	qint32 num = 0;
	float total = 0;
	
	query.prepare("SELECT cont.section,inv.paid_date,cont.amount,inv.address_city,cont.contribute_date,cont.year,inv.member_uid,inv.amount_paid"
	" FROM pps_contribution cont LEFT JOIN pps_invoice AS inv ON (cont.reference=inv.reference)"
	" WHERE cont.section=:section AND cont.contribute_date>=:begin AND cont.contribute_date<=:end AND cont.state=:state"
	" ORDER BY cont.section,cont.contribute_date,inv.paid_date ASC;");
	query.bindValue(":section", section);
	query.bindValue(":begin", from);
	query.bindValue(":end", to);
	query.bindValue(":state", (int)Invoice::StateContributed);
	query.exec();
	
	pdf->loadTemplate(settings.value("pdf/contribution_template", "data/contribution.xml").toString());
	while (query.next()) {
		entry = pdf->addEntry("payment");
		entry->setVar("num", ++num);
		entry->setVar("section", query.value(0).toString());
		entry->setVar("paid", query.value(1).toDate().toString(date_format));
		entry->setVar("amount", query.value(2).toFloat());
		entry->setVar("city", query.value(3).toString());
		entry->setVar("date", query.value(4).toDate().toString(date_format));
		entry->setVar("year", query.value(5).toInt());
		entry->setVar("member", query.value(6).toInt());
		entry->setVar("amount_paid", query.value(7).toFloat());

		total += query.value(2).toFloat();
	}
	pdf->setVar("section_name", section);
	pdf->setVar("contribution_year", QDate::currentDate().toString("yyyy"));
	pdf->setVar("contribution_number", QString::number(num));
	pdf->setVar("contribution_total", QString::number(total, 'f', 2));
	pdf->setVar("contribution_average", QString::number(total/num, 'f', 2));
	pdf->setVar("print_city", settings.value("pdf/var_print_city", "Vallorbe").toString());
	pdf->setVar("print_date", QDate::currentDate().toString(date_format));
	
	return pdf;
}

void Contributions::recalculateContributions() {
	QSettings settings;
	QString section = (recalcSection->currentIndex() > 0) ? recalcSection->currentText() : "";
	QDate valuta;
	int year = recalcYear->value();
	int num_contrib = 0;
	
	if (QMessageBox::warning(this, tr("Really recalculate contributions"), tr("Do you really want to recalculate all contributions from Section '%1' in year '%2'?\nAll current datasets will be removed and recreated.").arg(section).arg(year), QMessageBox::Ok | QMessageBox::Abort) == QMessageBox::Ok) {
		QSqlQuery query, query2;
		QStringList dontContribute = settings.value("contribution/dontpay", "members").toStringList();
		dontContribute.removeAll("");
		
		// Prepare the query for inserting contributions.
		query2.prepare("INSERT INTO pps_contribution (reference,state,section,amount,contribute_date,year) VALUES (:reference,:state,:section,:amount,:date,:year);");
		
		// Delete all existent contributions.
		query.prepare(QString("DELETE FROM pps_contribution WHERE year=:year %1;").arg( (recalcSection->currentIndex() == 0) ? "" : "AND section=:section" ));
		query.bindValue(":year", year);
		if (recalcSection->currentIndex() > 0) {
			query.bindValue(":section", section);
		}
		query.exec();
		
		// Get all contributions for the selected year.
		query = contributionsQuery(recalcPeriodFrom->date(), recalcPeriodUntil->date(), year, section, Invoice::StateContributed
#ifdef FIO
			, checkNoFIO->isChecked()
#endif
		);
		
		// Show a Progressbar
		QProgressDialog progress(tr("Recalculate and save contributions..."), tr("Please wait..."), 0, 1, this);
		progress.setWindowModality(Qt::WindowModal);
		progress.setValue(0);
		cancelProgress = FALSE;
		connect(&progress, SIGNAL(canceled()), this, SLOT(cancelRecreate()));
		
		// Cache the Quartal-Dates for checking the valuta against in the loop below
		QDate q1 = settings.value("payment/contribution_q1", QDate(year, 3, 31)).toDate();
		QDate q2 = settings.value("payment/contribution_q2", QDate(year, 6, 30)).toDate();
		QDate q3 = settings.value("payment/contribution_q3", QDate(year, 9, 30)).toDate();
		QDate q4 = settings.value("payment/contribution_q4", QDate(year, 12, 31)).toDate();
		
		progress.setMaximum(query.size());
		
#ifdef FIO
		if (checkNoFIO->isChecked()) {
#endif
		while (query.next() && !cancelProgress) {
			progress.setValue(++num_contrib);
			
			// Query-Fields: amount, amount_paid, recommendations, reference, paid_date, for_section
			// Get the valuta for the contribution
			valuta = query.value(4).toDate();
			if (valuta < q1) {
				valuta = recalcQ1->date();
			} else if (valuta < q2) {
				valuta = recalcQ2->date();
			} else if (valuta < q3) {
				valuta = recalcQ3->date();
			} else if (valuta < q4) {
				valuta = recalcQ4->date();
			} else {
				continue;
			}
			
			query2.bindValue(":section", query.value(5).toString());
			query2.bindValue(":date", valuta);
			query2.bindValue(":state", dontContribute.contains(query.value(2).toString()) ? Invoice::StateOpen : Invoice::StateContributed);
			query2.bindValue(":year", year);
			query2.bindValue(":reference", query.value(3).toString());
			query2.bindValue(":amount", query.value(0).toFloat() / 2.0);
			query2.exec();
		}
		progress.setMaximum(query.size() + 1);
		
#ifdef FIO
		} else {
			int progress_contrib = 0;
			bool do_contribute = FALSE;
			QList< contribution_data * > cdata;
			l_contrib_data.clear();
			calculateFioContribution(&cdata, &query, 1, 2, 3, 4);
			
			// For the status bar: Get the number of saving contributions
			for (int i = 0; i < cdata.size(); i++) {
				num_contrib += cdata.at(i)->amount_list.size();
				if (cancelProgress) break;
			}
			progress.setMaximum(num_contrib);
			
			// Fill the details with the calculated recommend values
			for (int i = 0; i < cdata.size(); i++) {
				contribution_data *cd = cdata.at(i);
				
				// If the process is cancelled, we need to clean up the data but not process them
				if (cancelProgress) {
					delete cd;
					continue;
				}
				section = cd->section;
				do_contribute = !dontContribute.contains(section);
				
				// Save all contribution parts into a separate table
				query2.bindValue(":section", cd->section);
				query2.bindValue(":state", do_contribute ? Invoice::StateContributed : Invoice::StateOpen);
				query2.bindValue(":year", year);
				for (int i = 0; i < cd->amount_list.size(); i++) {
					progress.setValue(++progress_contrib);
					
					// Get the valuta for the contribution
					valuta = cd->amount_list.at(i).valuta;
					if (valuta < q1) {
						valuta = recalcQ1->date();
					} else if (valuta < q2) {
						valuta = recalcQ2->date();
					} else if (valuta < q3) {
						valuta = recalcQ3->date();
					} else if (valuta < q4) {
						valuta = recalcQ4->date();
					} else {
						continue;
					}
					
					query2.bindValue(":reference", cd->amount_list.at(i).reference);
					query2.bindValue(":amount", QString::number(cd->amount_list.at(i).amount));
					query2.bindValue(":date", valuta);
					query2.exec();
				}
				delete cd;
			}
		}
#endif
		disconnect(&progress, SIGNAL(canceled()), this, SLOT(cancelRecreate()));
		progress.close();
	}
}

void Contributions::cancelRecreate() {
	cancelProgress = TRUE;
}

