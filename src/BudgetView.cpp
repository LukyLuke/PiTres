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
#include "budget_TreeItemDelegate.h"

#include <QDebug>
#include <QSettings>

BudgetView::BudgetView(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
	connect(actionRemoveFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
	connect(actionCreateRoot, SIGNAL(triggered()), this, SLOT(createRoot()));
	
	connect(actionCreateEntry, SIGNAL(triggered()), this, SLOT(createEntry()));
	connect(actionRemoveEntry, SIGNAL(triggered()), this, SLOT(removeEntry()));
	connect(actionChangeEntry, SIGNAL(triggered()), this, SLOT(editEntry()));
	
	treeModel = new budget::TreeModel(tr("#"), tr("Category"), tr("Description"), tr("Amount"), tr("Type"));
	treeView->setModel(treeModel);
	for (qint32 i = 0; i < treeModel->columnCount(); ++i) {
		treeView->resizeColumnToContents(i);
	}
	treeView->setItemDelegateForColumn(4, new budget::TreeItemDelegate());
	connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeClicked(QModelIndex)));
	connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(treeSelectionChanged()));
	
	listModel = new budget::BudgetEntityModel;
	listView->setModel(listModel);
	listView->setItemDelegate( new budget::BudgetEntityDelegate );
}

BudgetView::~BudgetView() {
	delete listModel;
	delete treeModel;
}

void BudgetView::createTables() {
	QSqlDatabase db;
	QSqlQuery query(db);
	query.exec("CREATE TABLE IF NOT EXISTS budget_tree (entity_id INTEGER PRIMARY KEY AUTOINCREMENT,position INTEGER, parent_id INTEGER, name TEXT, description TEXT, amount FLOAT, type INTEGER DEFAULT 0);");
	query.exec("CREATE INDEX IF NOT EXISTS parent_index ON budget_tree (parent_id);");
	query.exec("CREATE INDEX IF NOT EXISTS position_index ON budget_tree (position);");
}

QList <qint32>BudgetView::getChildSections(qint32 section) {
	return getChildSections(section, FALSE);
}

QList <qint32>BudgetView::getChildSections(qint32 section, bool childs) {
	QList<qint32> list;
	qint32 id;
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("SELECT entity_id FROM budget_tree WHERE parent_id=:section;");
	query.bindValue(":section", section);
	query.exec();
	while (query.next()) {
		id = query.value(0).toInt();
		list << id;
		if (childs) {
			list.append(getChildSections(id, TRUE));
		}
	}
	return list;
}

void BudgetView::treeClicked(const QModelIndex index) {
	budget::TreeItem *item = static_cast<budget::TreeItem *>(index.internalPointer());
	QList<BudgetEntity *> *list = BudgetEntity::getEntities(item->id(), TRUE);
	
	showSummary(list);
	listModel->setData(list);
}

void BudgetView::showSummary(QList<BudgetEntity *> *list) {
	qreal amount = 0;
	for (qint32 i = 0; i < list->size(); i++) {
		BudgetEntity *entity = list->at(i);
		amount += entity->amount();
	}
	labelTotal->setText("<b>" + locale.toCurrencyString(amount, locale.currencySymbol(QLocale::CurrencySymbol)) + "</b>");
}

void BudgetView::treeSelectionChanged() {
	if (treeView->selectionModel()->currentIndex().isValid()) {
		treeView->closePersistentEditor(treeView->selectionModel()->currentIndex());
	}
}

void BudgetView::createFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	qint32 row = 0;
	
	if (!treeModel->insertRow(row, sel)) {
		return;
	}
	
	// treeModel->data(treeModel->index(sel.row(), 0, sel), Qt::DisplayRole)
	treeModel->setData( treeModel->index(row, 0, sel), QVariant(0), Qt::EditRole );
	treeModel->setData( treeModel->index(row, 1, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(row, 2, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(row, 3, sel), QVariant(0.0), Qt::EditRole );
	
	treeView->selectionModel()->setCurrentIndex(treeModel->index(row, 0, sel), QItemSelectionModel::ClearAndSelect);
	treeSelectionChanged();
}

void BudgetView::createRoot() {
	treeView->selectionModel()->clear();
	createFolder();
}

void BudgetView::removeFolder() {
	// show Messagebox to confirm
	doRemoveFolder();
}

void BudgetView::doRemoveFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	
	if (!treeModel->removeRow(sel.row(), sel.parent())) {
		// Show Error
	}
}

void BudgetView::createEntry() {
	qDebug() << "Create Entry...";
}

void BudgetView::removeEntry() {
	// Show MessageBox to confirm
	qDebug() << "Remove Entry...";
}
void BudgetView::doRemoveEntry() {
	// remove...
}

void BudgetView::editEntry() {
	qDebug() << "Edit Entry...";
}
