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
	for (qint32 i = 0; i < treeModel->columnCount(); ++i) {
		treeView->resizeColumnToContents(i);
	}
	
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

void BudgetView::createRootFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	
	if (!treeModel->insertRow(0, sel)) {
		return;
	}
	
	treeModel->setData( treeModel->index(0, 0, sel), QVariant("0"), Qt::EditRole );
	treeModel->setData( treeModel->index(0, 1, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(0, 2, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(0, 3, sel), QVariant("0"), Qt::EditRole );
	
	treeView->selectionModel()->setCurrentIndex(treeModel->index(0, 0, sel), QItemSelectionModel::ClearAndSelect);
}

void BudgetView::createFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	qint32 idx = sel.row() + 1;
	
	qDebug() << treeView->selectionModel()->hasSelection();
	
	if (!treeModel->insertRow(idx, sel.parent())) {
		return;
	}
	
	//qDebug() << sel.row();
	//qDebug() << treeModel->index(sel.row(), 0, sel.parent());
	//qDebug() << treeModel->data(treeModel->index(sel.row(), 0, sel.parent()), Qt::DisplayRole);
	
	treeModel->setData( treeModel->index(idx, 0, sel), treeModel->data(treeModel->index(sel.row(), 0, sel.parent()), Qt::DisplayRole), Qt::EditRole );
	treeModel->setData( treeModel->index(idx, 1, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(idx, 2, sel), QVariant(tr("Enter Data")), Qt::EditRole );
	treeModel->setData( treeModel->index(idx, 3, sel), QVariant("0"), Qt::EditRole );
	
	//treeView->selectionModel()->setCurrentIndex(treeModel->index(idx, 0, sel), QItemSelectionModel::ClearAndSelect);
}

void BudgetView::removeFolder() {
	// show Messagebox to confirm
}

void BudgetView::doRemoveFolder() {
	QModelIndex sel = treeView->selectionModel()->currentIndex();
	
	if (!treeModel->removeRow(sel.row(), sel.parent())) {
		// Show Error
	}
}

void BudgetView::editFolder() {
	qDebug() << "Edit Folder...";
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


/**
* Internal Classes in a separate namespace
*/
namespace budget {
	
	/**
	* TreeView Itemts
	*/
	TreeItem::TreeItem(TreeItem *parent) {
		parentItem = parent;
		entityId = 0;
		itemData << "" << "" << "" << "";
	}

	TreeItem::~TreeItem() {
		qDeleteAll(childItems);
	}
	
	bool TreeItem::setData(const qint32 pos, const QVariant &data) {
		if (pos >= 0) {
			itemData[pos] = data;
			return true;
		}
		return false;
	}

	qint32 TreeItem::id() const {
		return entityId;
	}
	
	void TreeItem::setId(qint32 id) {
		entityId = id;
	}

	void TreeItem::appendChild(TreeItem *child) {
		childItems.append(child);
	}
	
	bool TreeItem::insertChildren(qint32 position, qint32 count) {
		if (position < 0 || position > childItems.size()) {
			return false;
		}
		for (qint32 row = 0; row < count; ++row) {
			TreeItem *item = new TreeItem(this);
			childItems.insert(position, item);
		}
		return true;
	}
	
	bool TreeItem::removeChildren(qint32 position, qint32 count) {
		if (position < 0 || position + count > childItems.size()) {
			return false;
		}
		for (qint32 row = 0; row < count; ++row) {
			delete childItems.takeAt(position);
		}
		return true;
	}

	TreeItem *TreeItem::child(qint32 row) {
		return childItems.value(row);
	}

	qint32 TreeItem::childCount() const {
		return childItems.count();
	}
	
	qint32 TreeItem::childNumber() const {
		if (parentItem) {
			return parentItem->childItems.indexOf(const_cast<TreeItem *>(this));
		}
		return 0;
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
		rootItem = new TreeItem();
		rootItem->setData(0, number);
		rootItem->setData(1, category);
		rootItem->setData(2, comment);
		rootItem->setData(3, amount);
		
		beginResetModel();
		setupModelData(rootItem);
		endResetModel();
	}
	
	TreeModel::~TreeModel() {
		delete rootItem;
	}
	
	QModelIndex TreeModel::index(qint32 row, qint32 column, const QModelIndex &parent) const {
		if (parent.isValid() && parent.column() != 0) {
			return QModelIndex();
		}
		
		TreeItem *parentItem = getItem(parent);
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
		
		TreeItem *childItem = getItem(index);
		TreeItem *parentItem = childItem->parent();
		
		if (!parentItem || parentItem == rootItem) {
			return QModelIndex();
		}
		return createIndex(parentItem->childNumber(), 0, parentItem);
	}
	
	qint32 TreeModel::rowCount(const QModelIndex &parent) const {
		TreeItem *parentItem = getItem(parent);
		return parentItem->childCount();
	}
	
	qint32 TreeModel::columnCount(const QModelIndex & /*parent*/) const {
		return rootItem->columnCount();
	}
	
	QVariant TreeModel::data(const QModelIndex &index, qint32 role) const {
		if (!index.isValid()) {
			return QVariant();
		}
		if (role != Qt::DisplayRole && role != Qt::EditRole) {
			return QVariant();
		}
		return getItem(index)->data(index.column());
	}
	
	TreeItem *TreeModel::getItem(const QModelIndex &index) const {
		if (index.isValid()) {
			TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
			if (item) {
				return item;
			}
		}
		return rootItem;
	}
	
	Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
		if (index.isValid()) {
			return 0;
		}
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	}
	
	bool TreeModel::setHeaderData(qint32 section, Qt::Orientation orientation, const QVariant &value, qint32 role) {
		if (role != Qt::EditRole || orientation != Qt::Horizontal) {
			return false;
		}
		bool success = rootItem->setData(section, value);
		if (success) {
			emit headerDataChanged(orientation, section, section);
		}
		return success;
	}
	
	QVariant TreeModel::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const {
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
			return rootItem->data(section);
		}
		return QVariant();
	}
	
	bool TreeModel::insertRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		TreeItem *parentItem = getItem(parent);
		bool success;
		
		beginInsertRows(parent, pos, pos + count - 1);
		success = parentItem->insertChildren(pos, count);
		endInsertRows();
		
		return success;
	}
	
	bool TreeModel::removeRows(qint32 pos, qint32 count, const QModelIndex &parent) {
		TreeItem *parentItem = getItem(parent);
		bool success;
		
		beginRemoveRows(parent, pos, pos + count - 1);
		success = parentItem->removeChildren(pos, count);
		endRemoveRows();
		
		return success;
	}
	
	bool TreeModel::setData(const QModelIndex &index, const QVariant &value, qint32 role) {
		if (role != Qt::EditRole) {
			return false;
		}
		TreeItem *item = getItem(index);
		bool success = item->setData(index.column(), value);
		if (success) {
			emit dataChanged(index, index);
		}
		return success;
	}
	
	void TreeModel::setupModelData(TreeItem *parent) {
		QLocale locale;
		QSqlQuery query(db);
		query.prepare("SELECT entity_id,name,description,amount FROM budget_tree WHERE parent_id=? ORDER BY entity_id;");
		query.bindValue(0, parent->id());
		query.exec();
		
		while (query.next()) {
			TreeItem *child = new TreeItem();
			child->setId(query.value(0).toInt());
			child->setData(0, query.value(0));
			child->setData(1, query.value(1));
			child->setData(2, query.value(2));
			child->setData(3, locale.toCurrencyString(query.value(3).toFloat(), locale.currencySymbol(QLocale::CurrencySymbol)));
			parent->appendChild(child);
			setupModelData(child);
		}
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
