/*
	The SectionEdit Widget is for editing the automatically created Sections
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

#ifndef SectionEdit_H
#define SectionEdit_H

#include "ui_editsections.h"
#include "data/Section.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QCompleter>
#include <QTimerEvent>
#include <QDebug>

class SectionEdit : public QWidget, private Ui::sectionEditForm {
Q_OBJECT

public:
	SectionEdit(QWidget *parent = 0);
	virtual ~SectionEdit();

private slots:
	void saveSection();
	void saveSectionTrigger();
	void resetSectionData();
	void showData(QModelIndex index);
	void reassignInvoices();
	
private:
	quint32 i_index;
	bool _loading;
	int saveTimer;
	QSqlDatabase db;
	QSqlQueryModel *listModel;
	QSqlQueryModel *userModel;
	QSqlQueryModel *reassignModel;
	QCompleter *completer;
	
	void initComboBoxes();
	void setReassignQuery();
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // SectionEdit_H
