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

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QTimerEvent>
#include <QHash>

class Contributions : public QWidget, private Ui::ContributionsForm {
Q_OBJECT

private:
#ifdef FIO
	struct contribution_data {
		QString section;
		float sum;
		QList<float> amount_list;
	};
	QList< contribution_data * > l_contrib_data;
#endif
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	QSqlQueryModel *contributionsModel;
	QHash<QString, QString> sectionQif;
	int searchTimer;
	
	QSqlQuery createQuery();
	void loadData();
	void showOverview();
	void createQif();
	void loadSectionContributions();
	QSqlQuery createContributionsQuery();
	void showContributionsDetails();
#ifdef FIO
	void calculateFioContribution( QList< contribution_data * > *cdata, QSqlQuery *query, int col_amount, int col_recom );
#endif

public:
	Contributions(QWidget *parent = 0);
	virtual ~Contributions();

public slots:
	void searchData();
	void searchDataTimeout();
	void exportData();
	void sendEmail();
	void selectSection();
	void selectYear();
	void searchContributions();
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // Contributions_H
