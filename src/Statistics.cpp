/*
	Statistical overview over all different things like invoices, members, etc.
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

#include "Statistics.h"

Statistics::Statistics(QWidget * parent) : QWidget(parent) {
	setupUi(this);
	
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportStatistic()));
	connect(btnPrint, SIGNAL(clicked()), this, SLOT(printStatistic()));
	connect(yearFrom, SIGNAL(valueChanged(int)), this, SLOT(startUpdateStatistic()));
	connect(yearTo, SIGNAL(valueChanged(int)), this, SLOT(startUpdateStatistic()));
	connect(sectionList, SIGNAL(currentIndexChanged(int)), this, SLOT(startUpdateStatistic()));
	
	statisticsArea->setScene(&scene);
	yearFrom->setValue(QDate::currentDate().year());
	yearTo->setValue(QDate::currentDate().year());
	loadSections();
	updateStatistic();
}

Statistics::~Statistics() { }

void Statistics::loadSections() {
	sectionList->addItem(tr("All"), QVariant(""));
	QSqlQuery query("SELECT name FROM pps_sections ORDER BY name;", db);
	while (query.next()) {
		sectionList->addItem(query.value(0).toString(), query.value(0));
	}
}

void Statistics::startUpdateStatistic() {
	killTimer(searchTimer);
	searchTimer = startTimer(500);
}

void Statistics::timerEvent(QTimerEvent * /*event*/) {
	killTimer(searchTimer);
	updateStatistic();
}

void Statistics::updateStatistic() {
	QSqlQuery query(db);
	QList<StatisticData> list;
	QDate begin(yearFrom->value(), 1, 1);
	QDate end(yearTo->value()+1, 1, 1);
	QString section = sectionList->currentText();
	
	currentY = 0;
	scene.clear();
	
	// Flip years if begin is greater than end
	if (yearFrom->value() > yearTo->value()) {
		yearFrom->setValue(end.year());
		yearTo->setValue(begin.year());
		end.setDate(yearTo->value()+1, 1, 1);
		begin.setDate(yearFrom->value(), 1, 1);
	}
	
	// TODO: Imlement the Section
	
	// Create an Entry for each year
	int years = end.year() - begin.year();
	for (int i = 0; i < years; i++) {
		QDate current(begin.year() + i, 12, 31);
		StatisticData d;
		d.section = "members";
		d.year = current.year();
		
		// Load Memberdata
		query.prepare("SELECT COUNT(uid) FROM ldap_persons WHERE joining<=:end;");
		query.bindValue(":end", current);
		d.members_num = query.exec() && query.first() ? query.value(0).toInt() : 0;
		
		query.prepare("SELECT COUNT(uid) FROM ldap_persons WHERE joining>:start AND joining<=:end;");
		query.bindValue(":start", QDate(current).addYears(-1));
		query.bindValue(":end", current);
		d.members_growth = query.exec() && query.first() ? query.value(0).toInt() : 0;
		d.members_growth_percent = 0;
		if (d.members_num > 0) {
			d.members_growth_percent = d.members_growth * 100 / d.members_num;
		}
		
		list.append(d);
		printData(d);
	}
}

void Statistics::exportStatistic() {
	
}

void Statistics::printStatistic() {
	
}

void Statistics::printData(Statistics::StatisticData data) {
	QGraphicsTextItem *item;
	int fs = 12;
	int lh = 17;
	
	QPen pen;
	pen.setWidthF(1.2);
	pen.setColor(Qt::black);
	
	QFont bold = QApplication::font();
	bold.setFamily("Aller light");
	bold.setWeight(QFont::Black);
	bold.setPixelSize(fs);
	
	QFont font = QApplication::font();
	font.setFamily("Aller light");
	font.setWeight(QFont::Normal);
	font.setPixelSize(fs);
	
	item = scene.addText(QString("Section \"%1\" in %2").arg(data.section).arg(data.year), bold);
	item->setPos(1, currentY);
	item->setDefaultTextColor(Qt::black);
	currentY += lh;
	
	scene.addLine(1, currentY + 5, 300, currentY + 5, pen);
	currentY += 10;
	
	item = scene.addText(tr("Num members:\t%1").arg(data.members_num), font);
	item->setPos(1, currentY);
	currentY += lh;
	
	item = scene.addText(tr("Members growth:\t%1  ( +%2\% )").arg(data.members_growth).arg(data.members_growth_percent), font);
	item->setPos(1, currentY);
	currentY += lh;
	
	
	currentY += lh;
}
