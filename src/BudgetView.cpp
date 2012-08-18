/*
	The BudgetView-Widget is for the Budget-Management and overview
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

#include "BudgetView.h"

#include <QDebug>

BudgetView::BudgetView(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
	connect(actionRemoveFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
	connect(actionChangeFolder, SIGNAL(triggered()), this, SLOT(editFolder()));
	
	connect(actionCreateEntry, SIGNAL(triggered()), this, SLOT(createEntry()));
	connect(actionRemoveEntry, SIGNAL(triggered()), this, SLOT(removeEntry()));
	connect(actionChangeEntry, SIGNAL(triggered()), this, SLOT(editEntry()));
	
}

BudgetView::~BudgetView() { }

void BudgetView::createFolder() {
	qDebug() << "Create Folder...";
}

void BudgetView::removeFolder() {
	qDebug() << "Remove Folder...";
}

void BudgetView::editFolder() {
	qDebug() << "Edit Folder...";
}

void BudgetView::createEntry() {
	qDebug() << "Create Entry...";
}

void BudgetView::removeEntry() {
	qDebug() << "Remove Entry...";
}

void BudgetView::editEntry() {
	qDebug() << "Edit Entry...";
}
