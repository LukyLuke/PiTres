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

#ifndef Statistics_H
#define Statistics_H

#include "ui_statistics.h"
#include "helper/XmlPdf.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QString>
#include <QDate>
#include <QLocale>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsTextItem>

#include <QDebug>

class Statistics : public QWidget, private Ui::StatisticsForm {
Q_OBJECT

private:
	struct StatisticData {
		QString section;
		qint32  year;
		qint32  members_num;
		qint32  members_growth;
		float   members_growth_percent;
		qint32  paid_num;
		qint32  paid_inc;
		float   paid_growth_percent;
		qint32  invoiced;
		qint32  invoiced_success;
		qint32  reminded;
		qint32  reminded_success;
	};
	
	QSqlDatabase db;
	QLocale locale;
	int searchTimer;
	QGraphicsScene scene;
	float currentY;
	
	void loadSections();
	void updateStatistic();
	void printData(StatisticData data);
	
public:
	Statistics(QWidget * parent = 0);
	virtual ~Statistics();
	
protected:
	void timerEvent(QTimerEvent *event);
	
private slots:
	void startUpdateStatistic();
	void exportStatistic();
	void printStatistic();
};

#endif // Statistics_H
