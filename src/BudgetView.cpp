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
#include <QSettings>
#include <QPushButton>

BudgetView::BudgetView(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	
	connect(actionCreateFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
	connect(actionRemoveFolder, SIGNAL(triggered()), this, SLOT(removeFolder()));
	connect(actionChangeFolder, SIGNAL(triggered()), this, SLOT(editFolder()));
	
	connect(actionCreateEntry, SIGNAL(triggered()), this, SLOT(createEntry()));
	connect(actionRemoveEntry, SIGNAL(triggered()), this, SLOT(removeEntry()));
	connect(actionChangeEntry, SIGNAL(triggered()), this, SLOT(editEntry()));
	
	connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(treeClicked(QModelIndex)));
	
	treeModel = new budget::TreeModel(tr("#"), tr("Category"), tr("Description"), tr("Amount"));
	treeView->setModel(treeModel);
	
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
	query.exec("CREATE TABLE IF NOT EXISTS budget_tree (entity_id INTEGER, parent_id INTEGER, name TEXT, description TEXT, amount FLOAT);");
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

void BudgetView::createFolder() {
	/*QSqlQuery query(db);
	QModelIndexList sel = treeView->selectedIndexes();
	if (!sel.isEmpty()) {
		QModelIndex idx = sel.at(0);
		budget::TreeItem *item = static_cast<budget::TreeItem *>(idx.internalPointer());
		
		query.prepare("INSERT INTO budget_tree (entity_id,parent_id,name,description) VALUES (:id,:parent,:name,:descr);");
		query.bindValue(":id", QString::number(item->id()) + "0");
		query.bindValue(":parent", item->id());
		query.bindValue(":name", "New Name");
		query.bindValue(":descr", "Description");
		query.exec();
		
		treeModel->insertRow(sel.at(0).row(), sel.at(0).parent());
	}*/
}

void BudgetView::removeFolder() {
	/*QModelIndexList sel = treeView->selectedIndexes();
	if (!sel.isEmpty()) {
		treeModel->removeRow(sel.at(0).row(), sel.at(0).parent());
	}*/
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


/**
 * Internal Classes in a separate namespace
 */
namespace budget {
	
	/**
	 * TreeView Itemts
	 */
	TreeItem::TreeItem(const QVariant &data, const qint32 id, TreeItem *parent) {
		parentItem = parent;
		itemData << data;
		entityId = id;
	}

	TreeItem::~TreeItem() {
		qDeleteAll(childItems);
	}
	
	void TreeItem::setComment(const QVariant &data) {
		itemData << data;
	}

	qint32 TreeItem::id() const {
		return entityId;
	}

	void TreeItem::appendChild(TreeItem *child) {
		childItems.append(child);
	}

	TreeItem *TreeItem::child(qint32 row) {
		return childItems.value(row);
	}

	qint32 TreeItem::childCount() const {
		return childItems.count();
	}

	qint32 TreeItem::columnCount() const {
		return itemData.count();
	}

	QVariant TreeItem::data(qint32 column) const {
		return itemData.value(column);
	}

	qint32 TreeItem::row() const {
		if (parentItem) {
			return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
		}
		return 0;
	}

	TreeItem* TreeItem::parent() {
		return parentItem;
	}

	/**
	 * The Model implementation for the TreeView
	 */
	TreeModel::TreeModel(const QString number, const QString category, const QString comment, const QString amount, QObject *parent) : QAbstractItemModel(parent) {
		rootItem = new TreeItem(number, 0);
		rootItem->setComment(category);
		rootItem->setComment(comment);
		rootItem->setComment(amount);
		setupModelData(rootItem);
	}
	
	TreeModel::~TreeModel() {
		delete rootItem;
	}
	
	QModelIndex TreeModel::index(qint32 row, qint32 column, const QModelIndex &parent) const {
		if (!hasIndex(row, column, parent)) {
			return QModelIndex();
		}
		
		TreeItem *parentItem;
		if (!parent.isValid()) {
			parentItem = rootItem;
		} else {
			parentItem = static_cast<TreeItem*>(parent.internalPointer());
		}
		
		TreeItem *childItem = parentItem->child(row);
		if (childItem) {
			return createIndex(row, column, childItem);
		}
		return QModelIndex();
	}
	
	QModelIndex TreeModel::parent(const QModelIndex &index) const {
		if (!index.isValid()) {
			return QModelIndex();
		}
		
		TreeItem *childItem = static_cast<TreeItem *>(index.internalPointer());
		TreeItem *parentItem = childItem->parent();
		
		if (parentItem == rootItem) {
			return QModelIndex();
		}
		return createIndex(parentItem->row(), 0, parentItem);
	}
	
	qint32 TreeModel::rowCount(const QModelIndex &parent) const {
		TreeItem *parentItem;
		if (parent.column() > 0) {
			return 0;
		}
		
		if (!parent.isValid()) {
			parentItem = rootItem;
		} else {
			parentItem = static_cast<TreeItem *>(parent.internalPointer());
		}
		return parentItem->childCount();
	}
	
	qint32 TreeModel::columnCount(const QModelIndex &parent) const {
		if (parent.isValid()) {
			return static_cast<TreeItem *>(parent.internalPointer())->columnCount();
		}
		return rootItem->columnCount();
	}

	QVariant TreeModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		
		TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
		return item->data(index.column());
	}
	
	Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
		if (index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
	
	QVariant TreeModel::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			return rootItem->data(section);
		}
		return QVariant();
	}
	
	bool TreeModel::removeRows(int row, int count, const QModelIndex &parent) {
		QSqlQuery query(db);
		query.prepare("");
		return QAbstractItemModel::removeRows(row, count, parent);
	}
	
	void TreeModel::setupModelData(TreeItem *parent) {
		QLocale locale;
		QSqlQuery query(db);
		query.prepare("SELECT entity_id,name,description,amount FROM budget_tree WHERE parent_id=? ORDER BY entity_id;");
		query.bindValue(0, parent->id());
		query.exec();
		
		beginResetModel();
		while (query.next()) {
			TreeItem *child = new TreeItem( query.value(0).toString(), query.value(0).toInt(), parent);
			child->setComment(query.value(1));
			child->setComment(query.value(2));
			child->setComment(locale.toCurrencyString(query.value(3).toFloat(), locale.currencySymbol(QLocale::CurrencySymbol)));
			setupModelData(child);
			parent->appendChild(child);
		}
		endResetModel();
	}
	
	/**
	 * The Model implementation for the ListView
	 */
	BudgetEntityModel::BudgetEntityModel(QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
	}
	BudgetEntityModel::BudgetEntityModel(QList<BudgetEntity *> *items, QObject * parent) : QAbstractListModel(parent) {
		entities = new QList<BudgetEntity *>();
		setData(items);
	}
	
	BudgetEntityModel::~BudgetEntityModel() {
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		delete entities;
	}
	
	void BudgetEntityModel::setData(QList<BudgetEntity *> *items) {
		beginResetModel();
		while (entities && !entities->isEmpty()) {
			BudgetEntity *ent = entities->takeFirst();
			delete ent;
		}
		entities = items;
		endResetModel();
	}
	
	qint32 BudgetEntityModel::rowCount(const QModelIndex &parent) const {
		if (entities) {
			return entities->size();
		}
		return 0;
	}

	QVariant BudgetEntityModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid() || role != Qt::DisplayRole) {
			return QVariant();
		}
		if (!entities || entities->size() <= index.row()) {
			return QVariant();
		}
		
		BudgetEntity *entity = entities->at(index.row());
		QVariant ret;
		ret.setValue<BudgetEntity>(*entity);
		return ret;
	}
	
	
	BudgetEntityDelegate::BudgetEntityDelegate(QObject * parent) : QStyledItemDelegate(parent) {
		dateWidth = 0;
	}
	
	void BudgetEntityDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		if (index.column() != 0 || !index.data().canConvert<BudgetEntity>()) {
			QStyledItemDelegate::paint(painter, option, index);
			return;
		}
		
		BudgetEntity entity = qvariant_cast<BudgetEntity>(index.data());
		
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing);
		
		QLocale locale;
		QStyleOptionViewItemV4 opt = option;
		QStyledItemDelegate::initStyleOption(&opt, index);
		QRect rect = opt.rect;
		
		opt.text = "";
		QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
		style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
		
		QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
		
		if (opt.state & QStyle::State_Selected) {
			painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
		} else {
			painter->setPen(opt.palette.color(cg, QPalette::Text));
		}
		
		if (dateWidth <= 0) {
			QDate _date = QDate(9999, 12, 29);
			*((int*)(&dateWidth)) = painter->fontMetrics().width(_date.toString(locale.dateFormat(QLocale::ShortFormat))) + 10;
		}
		
		QString amount = locale.toCurrencyString(entity.amount(), locale.currencySymbol(QLocale::CurrencySymbol));
		int amountWidth = painter->fontMetrics().width(amount);
		QRect adj = rect.adjusted(3, rect.height()/3, -3, rect.height()/3);
		
		painter->drawText(adj.adjusted(0, 0, -amountWidth, 0), Qt::TextSingleLine, entity.date().toString(locale.dateFormat(QLocale::ShortFormat)));
		painter->drawText(adj.adjusted(adj.left() + dateWidth, 0, -amountWidth, 0), Qt::TextSingleLine, entity.description());
		painter->drawText(adj.adjusted(adj.left() + dateWidth + amountWidth, 0, 0, 0), Qt::AlignRight | Qt::TextSingleLine, amount);
		
		painter->restore();
	}
	
	QSize BudgetEntityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(size.height() * 3);
		return size;
	}
	
}
