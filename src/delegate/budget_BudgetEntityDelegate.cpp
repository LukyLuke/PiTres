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

#include "budget_BudgetEntityDelegate.h"
#include <QHBoxLayout>
#include <QDateEdit>
#include <QPlainTextEdit>
#include <QDoubleSpinBox>

namespace budget {

	BudgetEntityDelegate::BudgetEntityDelegate(QObject *parent) : QStyledItemDelegate(parent) {
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
			//*((int*)(&dateWidth)) = painter->fontMetrics().width(_date.toString(locale.dateFormat(QLocale::ShortFormat))) + 10;
			dateWidth = painter->fontMetrics().width(_date.toString(locale.dateFormat(QLocale::ShortFormat))) + 10;;
		}
		
		QString amount = locale.toCurrencyString(entity.amount(), locale.currencySymbol(QLocale::CurrencySymbol));
		int amountWidth = painter->fontMetrics().width(amount);
		QRect adj = rect.adjusted(3, rect.height()/3, -3, rect.height()/3);
		
		painter->drawText(adj.adjusted(0, 0, -amountWidth, 0), Qt::TextSingleLine, entity.date().toString(locale.dateFormat(QLocale::ShortFormat)));
		painter->drawText(adj.adjusted(adj.left() + dateWidth, 0, -amountWidth, 0), Qt::TextWordWrap, entity.description());
		painter->drawText(adj.adjusted(adj.left() + dateWidth + amountWidth, 0, 0, 0), Qt::AlignRight | Qt::TextSingleLine, amount);
		
		painter->restore();
	}
	
	QSize BudgetEntityDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
		QSize size = QStyledItemDelegate::sizeHint(option, index);
		size.setHeight(size.height() * 3);
		return size;
	}
	
	QWidget *BudgetEntityDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const {
		QLocale locale;
		QWidget *editor = new QWidget(parent);
		QHBoxLayout *lay = new QHBoxLayout;
		QDateEdit *dateEdit = new QDateEdit(QDate::currentDate());
		QPlainTextEdit *textEdit = new QPlainTextEdit("Textedig");
		QDoubleSpinBox *spinBox = new QDoubleSpinBox;
		
		dateEdit->setDisplayFormat("yyyy-MM-dd");
		dateEdit->setCalendarPopup(true);
		
		spinBox->setMaximum(9999999.99);
		spinBox->setPrefix(locale.currencySymbol(QLocale::CurrencySymbol));
		spinBox->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
		
		editor->setLayout(lay);
		lay->addWidget(dateEdit);
		lay->addWidget(textEdit);
		lay->addWidget(spinBox);
		
		return editor;
	}
	
	void BudgetEntityDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
		BudgetEntity entity = qvariant_cast<BudgetEntity>(index.data());
		
		QDateEdit *dateEdit = qobject_cast<QDateEdit *>( editor->layout()->itemAt(0)->widget() );
		QPlainTextEdit *textEdit = qobject_cast<QPlainTextEdit *>( editor->layout()->itemAt(1)->widget() );
		QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>( editor->layout()->itemAt(2)->widget() );
		
		dateEdit->setDate(entity.date());
		textEdit->setPlainText(entity.description());
		spinBox->setValue(entity.amount());
	}
	
	void BudgetEntityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
		BudgetEntity entity = qvariant_cast<BudgetEntity>(index.data());
		
		QDateEdit *dateEdit = qobject_cast<QDateEdit *>( editor->layout()->itemAt(0)->widget() );
		QPlainTextEdit *textEdit = qobject_cast<QPlainTextEdit *>( editor->layout()->itemAt(1)->widget() );
		QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>( editor->layout()->itemAt(2)->widget() );
		
		entity.setDate(dateEdit->date());
		entity.setDescription(textEdit->toPlainText());
		entity.setAmount(spinBox->value());
		
		QVariant val;
		val.setValue<BudgetEntity>(entity);
		model->setData(index, val, Qt::EditRole);
	}
	
	void BudgetEntityDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const {
		editor->setGeometry(option.rect);
	}
	
}
