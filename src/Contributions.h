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

#ifndef Contributions_H
#define Contributions_H

#include "ui_contributions.h"
#include "helper/FIOCalc.h"
#include "data/Section.h"
#include "data/Invoice.h"
#include "data/Person.h"
#include "helper/Smtp.h"
#include "helper/XmlPdf.h"

#include <cstdlib>
#include <cstdio>
#include <QFileDialog>
#include <QFile>
#include <QList>
#include <QSettings>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QTimerEvent>
#include <QPair>
#include <QHash>
#include <QDate>
#include <QProgressDialog>

class Contributions : public QWidget, private Ui::ContributionsForm {
Q_OBJECT

private:
#ifdef FIO
	struct contribution_data_invoice {
		QString reference;
		float amount;
		QDate valuta;
	};
	struct contribution_data {
		QString section;
		float sum;
		QList< contribution_data_invoice > amount_list;
	};
	QList< contribution_data * > l_contrib_data;
#endif
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	QSqlQueryModel *contributionsModel;
	QHash<QString, QString> sectionQif;
	int searchTimer;
	bool cancelProgress;
	
	QSqlQuery createTableModelQuery();
	void loadData();
	void showOverview();
	void createQif();
	void loadSectionContributions();
	QSqlQuery createContributionsQuery();
	QSqlQuery contributionsQuery(QDate from, QDate until, int payable_year, QString section = "", Invoice::State state = Invoice::StatePaid, bool noFioCalculation = FALSE);
	void showBookedContributions();
	void fillContributionDateList(int year);
	XmlPdf * createContributionsPdf(QString section, bool showAll = FALSE);
#ifdef FIO
	void calculateFioContribution( QList< contribution_data * > *cdata, QSqlQuery *query, int col_amount, int col_recom, int col_reference, int col_valuta);
#endif

public:
	Contributions(QWidget *parent = 0);
	virtual ~Contributions();
	XmlPdf * getPdf(const QString &section, const QDate &from, const QDate &to) const;

public slots:
	void searchData();
	void searchDataTimeout();
	void exportData();
	void sendEmail();
	void selectSection();
	void selectYear();
	void selectDate();
	void searchContributions();
	void sendContributionDetails();
	void exportContributionDetails();
	void recalculateContributions();
	void cancelRecreate();
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // Contributions_H
