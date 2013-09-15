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

	connect(yearFrom, SIGNAL(valueChanged(int)), this, SLOT(startUpdateStatistic()));
	connect(yearTo, SIGNAL(valueChanged(int)), this, SLOT(startUpdateStatistic()));
	connect(sectionList, SIGNAL(currentIndexChanged(int)), this, SLOT(startUpdateStatistic()));
	
	connect(btnPrint, SIGNAL(clicked()), this, SLOT(printStatistic()));
	connect(btnExport, SIGNAL(clicked()), this, SLOT(exportStatistic()));
	connect(btnCsv, SIGNAL(clicked()), this, SLOT(exportStatisticAsCsv()));

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
	QDate begin(yearFrom->value(), 1, 1);
	QDate end(yearTo->value(), 12, 31);
	QString section = sectionList->currentText();

	currentY = 0;
	scene.clear();
	list.clear();

	// Flip years if begin is greater than end
	if (yearFrom->value() > yearTo->value()) {
		yearFrom->setValue(end.year());
		yearTo->setValue(begin.year());
		end.setDate(yearTo->value(), 12, 31);
		begin.setDate(yearFrom->value(), 1, 1);
	}

	// TODO: Implement sections

	// Create an Entry for each year
	int years = end.year() - begin.year() + 1;
	for (int i = 0; i < years; i++) {
		QDate current(begin.year() + i, 12, 31);
		QDate start(begin.year() + i, 1, 1);
		QDate last(begin.year() + i - 1, 1, 1);
		StatisticData d;
		d.section = "members";
		d.year = current.year();

		// Load Memberdata
		query.prepare("SELECT COUNT(uid) FROM ldap_persons WHERE joining<=:end;");
		query.bindValue(":end", current);
		d.members_num = query.exec() && query.first() ? query.value(0).toInt() : 0;

		query.prepare("SELECT COUNT(uid) FROM ldap_persons WHERE joining>=:start AND joining<=:end;");
		query.bindValue(":start", start);
		query.bindValue(":end", current);
		d.members_growth = query.exec() && query.first() ? query.value(0).toInt() : 0;
		d.members_growth_percent = 0;
		if (d.members_num > 0) {
			d.members_growth_percent = d.members_growth / d.members_num;
		}
		d.members_growth_percent *= 100.0;

		// Load number of paid invoices
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE paid_date>=:start AND paid_date<=:end;");
		query.bindValue(":start", start);
		query.bindValue(":end", current);
		d.paid_num = query.exec() && query.first() ? query.value(0).toInt() : 0;
		d.paid_percent = 0;
		if (d.members_num > 0) {
			d.paid_percent = d.paid_num / d.members_num;
		}
		d.paid_percent *= 100.0;

		// Last year to get the increase of paid invoices
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE paid_date>=:start AND paid_date<:end;");
		query.bindValue(":start", last);
		query.bindValue(":end", start);
		d.paid_inc = query.exec() && query.first() ? d.paid_num - query.value(0).toInt() : 0;
		d.paid_growth_percent = 0;
		if (d.paid_num > 0) {
			d.paid_growth_percent = d.paid_inc / d.paid_num;
		}
		d.paid_growth_percent *= 100.0;

		// Load number of issued invoices
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE issue_date>:start AND issue_date<=:end;");
		query.bindValue(":start", start);
		query.bindValue(":end", current);
		d.invoiced = query.exec() && query.first() ? query.value(0).toInt() : 0;

		// Last year to get the increase of issued invoices
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE issue_date>=:start AND issue_date<:end AND amount_paid>0 AND reminded<=0;");
		query.bindValue(":start", last);
		query.bindValue(":end", start);
		d.invoiced_success = query.exec() && query.first() ? query.value(0).toInt() : 0;
		d.invoiced_percent = 0;
		if (d.invoiced > 0) {
			d.invoiced_percent = d.invoiced_success / d.invoiced;
		}
		d.invoiced_percent *= 100.0;

		// Load all reminders
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE issue_date>:start AND issue_date<=:end AND reminded>0;");
		query.bindValue(":start", start);
		query.bindValue(":end", current);
		d.reminded = query.exec() && query.first() ? query.value(0).toInt() : 0;

		// Last year to get the increase of reminders
		query.prepare("SELECT COUNT(reference) FROM pps_invoice WHERE issue_date>=:start AND issue_date<:end AND amount_paid>0 AND reminded>0;");
		query.bindValue(":start", last);
		query.bindValue(":end", start);
		d.reminded_success = query.exec() && query.first() ? query.value(0).toInt() : 0;
		d.reminded_percent = 0;
		if (d.reminded > 0) {
			d.reminded_percent = d.reminded_success / d.reminded;
		}
		d.reminded_percent *= 100.0;

		// Load the total and paid amount
		query.prepare("SELECT SUM(amount), SUM(amount_paid) FROM pps_invoice WHERE issue_date>:start AND issue_date<=:end;");
		query.bindValue(":start", start);
		query.bindValue(":end", current);
		d.amount_requested = query.exec() && query.first() ? query.value(0).toFloat() : 0;
		d.amount_paid = query.exec() && query.first() ? query.value(1).toFloat() : 0;

		list.append(d);
		printData(d);
	}
}

void Statistics::printStatistic() {
	QPrinter printer(QPrinter::HighResolution);
	printer.setPaperSize(QPrinter::A4);
	printer.setOrientation(QPrinter::Portrait);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setPageMargins(3.53, 3.53, 3.53, 3.53, QPrinter::Millimeter);
	
	QPrintDialog *dialog = new QPrintDialog(&printer);
	dialog->setWindowTitle(QObject::tr("Print Document"));
	if (dialog->exec() != QDialog::Accepted) {
		qDebug() << "Printing aborted and no Filename given, canceled printing...";
		return;
	}
	
	QPainter *painter = new QPainter();
	if (!painter->begin(&printer)) {
		qDebug() << "Error while write to File/Printer";
		return;
	}
	// print out here
}

void Statistics::exportStatistic() {
	QSettings settings;
	XmlPdf pdf;
	XmlPdfEntry *entry;
	QString fileName;
	
	fileName = QFileDialog::getSaveFileName(this, tr("Save Statistics as PDF"), "", tr("PDF (*.pdf)"));
	if (fileName.isEmpty()) {
		return;
	}
	
	QString templateFile = settings.value(QString("pdf/statistic_template"), "data/statistic_de.xml").toString();
	templateFile.replace(QRegExp("^(.*_)([a-zA-Z]{2})(\\.xml)$"), "\\1de\\3"); // TODO: Use short language of the user settings here and not only german...
	pdf.loadTemplate(templateFile);
	
	QList<StatisticData>::const_iterator it;
	for (it = list.constBegin(); it != list.constEnd(); it++) {
		entry = pdf.addEntry("entry");
		entry->setVar("section", it->section);
		entry->setVar("year", it->year);
		entry->setVar("members_num", it->members_num);
		entry->setVar("members_growth", it->members_growth);
		entry->setVar("members_growth_percent", it->members_growth_percent);
		entry->setVar("paid_num", it->paid_num);
		entry->setVar("paid_inc", it->paid_inc);
		entry->setVar("paid_percent", it->paid_percent);
		entry->setVar("amount_requested", it->amount_requested);
		entry->setVar("amount_paid", it->amount_paid);
		entry->setVar("paid_growth_percent", it->paid_growth_percent);
		entry->setVar("invoiced", it->invoiced);
		entry->setVar("invoiced_success", it->invoiced_success);
		entry->setVar("invoiced_percent", it->invoiced_percent);
		entry->setVar("reminded", it->reminded);
		entry->setVar("reminded_success", it->reminded_success);
		entry->setVar("reminded_percent", it->reminded_percent);
	}
	pdf.print(fileName);
}

void Statistics::exportStatisticAsCsv() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Statistics as CSV"), "", tr("CSV (*.csv);;Plaintext (*.txt)"));
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Truncate)) {
			QTextStream out(&f);
			
			QList<StatisticData>::const_iterator it;
			for (it = list.constBegin(); it != list.constEnd(); it++) {
				out << tr("Section") << ";" << it->section << ";" << it->year << "\n";
				out << tr("Members number/growth") << ";" << it->members_num << ";" << it->members_growth << ";" << it->members_growth_percent/100 << "\n";
				out << tr("Amount requested/paid") << ";" << it->amount_requested << ";" << it->amount_paid << ";" << it->paid_percent/100 << "\n";
				out << tr("Payments number/growth") << ";" << it->paid_num << ";" << it->paid_inc << ";" << it->paid_growth_percent/100 << "\n";
				out << tr("Invoices number/paid") << ";" << it->invoiced << ";" << it->invoiced_success << ";" << it->invoiced_percent/100 << "\n";
				out << tr("Reminders number/paid") << ";" << it->reminded << ";" << it->reminded_success << ";" << it->reminded_percent/100 << "\n";
				out << "\n";
			}
		}
	}
}

void Statistics::printData(Statistics::StatisticData data) {
	int prec = 4;
	QChar ch(' ');
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

	scene.addLine(1, currentY + 5, 280, currentY + 5, pen);
	currentY += 10;

	item = scene.addText(tr("Num members:\t%L1").arg(data.members_num), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Members growth:\t%L1  ( %2\% )").arg(data.members_growth).arg(data.members_growth_percent, 0, 'g', prec), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Total payments:\t%L1  ( %2\% )").arg(data.paid_num).arg(data.paid_percent, 0, 'g', prec), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Amount requested:\t%1").arg( locale.toCurrencyString(data.amount_requested) ), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Total amount paid:\t%1").arg( locale.toCurrencyString(data.amount_paid) ), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Payment growth:\t%L1  ( %2\% )").arg(data.paid_inc).arg(data.paid_growth_percent, 0, 'g', prec), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Number invoices:\t%L1 of %2 paid  ( %3\% )").arg(data.invoiced_success).arg(data.invoiced).arg(data.invoiced_percent, 0, 'g', prec), font);
	item->setPos(1, currentY);
	currentY += lh;

	item = scene.addText(tr("Number reminders:\t%L1 of %2 paid  ( %3\% )").arg(data.reminded_success).arg(data.reminded).arg(data.reminded_percent, 0, 'g', prec), font);
	item->setPos(1, currentY);
	currentY += lh;

	currentY += lh;
}
