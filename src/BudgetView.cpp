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

BudgetView::BudgetView(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
	connect(actionRemoveFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
	connect(actionCreateRoot, SIGNAL(triggered()), this, SLOT(createRoot()));
	
	connect(actionCreateEntry, SIGNAL(triggered()), this, SLOT(createEntry()));
	connect(actionRemoveEntry, SIGNAL(triggered()), this, SLOT(removeEntry()));
	
	treeModel = new budget::TreeModel(tr("#"), tr("Name"), tr("Description"), tr("Amount"), tr("Type"));
	treeView->setModel(treeModel);
	for (qint32 i = 0; i < treeModel->columnCount(); ++i) {
		treeView->resizeColumnToContents(i);
	}
	treeView->setItemDelegateForColumn(3, new budget::TreeItemAmountDelegate);
	treeView->setItemDelegateForColumn(4, new budget::TreeItemTypeDelegate);
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

QList<qint32> BudgetView::getChildSections(qint32 section) {
	return getChildSections(section, FALSE);
}

QList<qint32> BudgetView::getChildSections(qint32 section, bool childs) {
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

QList<qint32> BudgetView::getParentSections(qint32 section) {
	return getParentSections(section, FALSE);
}

QList<qint32> BudgetView::getParentSections(qint32 section, bool childs) {
	QList<qint32> list;
	qint32 id;
	QSqlDatabase db;
	QSqlQuery query(db);
	query.prepare("SELECT parent_id FROM budget_tree WHERE entity_id=:section;");
	query.bindValue(":section", section);
	query.exec();
	while (query.next()) {
		id = query.value(0).toInt();
		if (id > 0) {
			list << id;
			if (childs) {
				list.append(getParentSections(id, TRUE));
			}
		}
	}
	return list;
}

void BudgetView::treeClicked(const QModelIndex index) {
	budget::TreeItem *item = static_cast<budget::TreeItem *>(index.internalPointer());
	QList<BudgetEntity *> *list = BudgetEntity::getEntities(item->entityId(), TRUE);
	
	showSummary(list, item);
	listModel->setData(item->entityId(), list);
}

void BudgetView::showSummary(QList<BudgetEntity *> *list, budget::TreeItem *item) {
	qreal spent = 0, budgeted = 0, spentCurrent = 0;
	QList<qint32> parents = getParentSections(item->entityId(), TRUE);
	QList<qint32> childs = getChildSections(item->entityId(), TRUE);
	budget::TreeItem *treeItem;
	qint32 current = item->entityId();
	
	// Calculate the spent amount
	for (qint32 i = 0; i < list->size(); i++) {
		BudgetEntity *entity = list->at(i);
		spent += entity->amount();
		if (entity->section() == current) {
			spentCurrent += entity->amount();
		}
	}
	labelTotal->setText("<b>" + locale.toCurrencyString(spent, locale.currencySymbol(QLocale::CurrencySymbol)) + "</b>");
	
	// Show the Details
	budgeted += item->data(3).toFloat();
	for (qint32 i = 0; i < childs.size(); ++i) {
		treeItem = treeModel->search(childs.at(i));
		budgeted += treeItem->data(3).toFloat();
	}
	
	if (!parents.empty()) {
		treeItem = treeModel->search(parents.last());
		labelSummaryBudget->setText(treeItem->data(1).toString());
		
	} else {
		labelSummaryBudget->setText(tr("Unknown Budget"));
	}
	labelSummaryPosition->setText(item->data(1).toString());
	labelSummaryBudgeted->setText(locale.toCurrencyString(budgeted, locale.currencySymbol(QLocale::CurrencySymbol)));
	labelSummarySpent->setText(locale.toCurrencyString(spent, locale.currencySymbol(QLocale::CurrencySymbol)));
	labelSummaryTotal->setText(locale.toCurrencyString(budgeted - spent, locale.currencySymbol(QLocale::CurrencySymbol)));
	
	labelOverviewPosition->setText(item->data(1).toString());
	labelOverviewBudgeted->setText(locale.toCurrencyString(item->data(3).toFloat(), locale.currencySymbol(QLocale::CurrencySymbol)));
	labelOverviewSpent->setText(locale.toCurrencyString(spentCurrent, locale.currencySymbol(QLocale::CurrencySymbol)));
	labelOverviewTotal->setText(locale.toCurrencyString(item->data(3).toFloat() - spentCurrent, locale.currencySymbol(QLocale::CurrencySymbol)));
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
	int ret = QMessageBox::question(this, tr("Delete Folder"), tr("Really delete the selected Position and all Subpositions?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Ok) {
		doRemoveFolder();
	}
}

void BudgetView::doRemoveFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	if (!treeModel->removeRow(sel.row(), sel.parent())) {
		QMessageBox::critical(this, tr("Delete failed"), tr("Could not delete the Position. Try it again or let it be..."));
	}
}

void BudgetView::createEntry() {
	QModelIndex sel = listView->selectionModel()->currentIndex();
	qint32 row = sel.row() + 1;
	
	if (!listModel->insertRow(row, sel)) {
		return;
	}
	
	listView->selectionModel()->setCurrentIndex( listModel->index(row, 0, sel.parent()), QItemSelectionModel::ClearAndSelect );
	listView->closePersistentEditor( listView->selectionModel()->currentIndex() );
}

void BudgetView::removeEntry() {
	int ret = QMessageBox::question(this, tr("Delete Entry"), tr("Really delete the Selected Entry?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
	if (ret == QMessageBox::Ok) {
		doRemoveEntry();
	}
}

void BudgetView::doRemoveEntry() {
	QModelIndex sel = listView->selectionModel()->currentIndex();
	if (!listModel->removeRow(sel.row(), sel.parent())) {
		QMessageBox::critical(this, tr("Failed to remove"), tr("Error while remove the Budget-Entity. Try again or let it be..."));
	}
}

