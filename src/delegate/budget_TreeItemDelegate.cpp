/*
    <one line to give the program's name and a brief idea of what it does.>
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


#include "budget_TreeItemDelegate.h"

namespace budget {
	TreeItemDelegate::TreeItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {
	}
	
	void TreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		qint32 value = index.model()->data(index, Qt::EditRole).toInt();
		QStyleOptionViewItemV4 opt = option;
		QStyledItemDelegate::initStyleOption(&opt, index);
		
		painter->save();
		QString s;
		switch (value) {
			case 0: s = tr("Budget"); break;
			case 1: s = tr("Expense"); break;
			case 2: s = tr("Income"); break;
			default: s = tr("Unknown"); break;
		}
		painter->drawText(opt.rect, Qt::TextSingleLine, s);
		painter->restore();
	}
	
	QWidget *TreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		QComboBox *box = new QComboBox(parent);
		box->addItem(tr("Budget"), QVariant(0));
		box->addItem(tr("Expense"), QVariant(1));
		box->addItem(tr("Income"), QVariant(2));
		return box;
	}
	
	void TreeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
		qint32 value = index.model()->data(index, Qt::EditRole).toInt();
		QComboBox *box = static_cast<QComboBox *>(editor);
		box->setCurrentIndex( box->findData(value) );
	}
	
	void TreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
		QComboBox *box = static_cast<QComboBox *>(editor);
		int value = box->itemData( box->currentIndex() ).toInt();
		model->setData(index, value, Qt::EditRole);
	}
	
	void TreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		editor->setGeometry(option.rect);
	}
	
}