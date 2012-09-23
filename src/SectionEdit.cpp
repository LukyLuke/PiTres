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

#include "SectionEdit.h"

SectionEdit::SectionEdit(QWidget *parent) : QWidget(parent) {
	setupUi(this);
	i_index = 0;
	
	connect(btnSave, SIGNAL(clicked()), this, SLOT(saveSection()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(resetSectionData()));
	connect(sectionList, SIGNAL(clicked(QModelIndex)), this, SLOT(showData(QModelIndex)));
	
	QSqlQuery query(db);
	query.exec("SELECT name || ', ' || description || ' (' || parent || ')' AS label, name FROM pps_sections ORDER BY parent,name ASC;");
	listModel = new QSqlQueryModel;
	listModel->setQuery(query);
	sectionList->setModel(listModel);
	sectionList->setModelColumn(0);
	
	initComboBoxes();
}

SectionEdit::~SectionEdit() {
	delete listModel;
	delete userModel;
}

void SectionEdit::initComboBoxes() {
	// The Parent-Combobox
	QSqlQuery query(db);
	query.exec("SELECT name || ', ' || description || ' (' || parent || ')' AS label, name FROM pps_sections ORDER BY parent,name ASC;");
	while (query.next()) {
		editParent->addItem(query.value(0).toString(), query.value(1).toString());
	}
	
	// Treasurers
	query.exec("SELECT givenname || ' ' || familyname AS name, uid FROM ldap_persons ORDER BY name ASC;");
	userModel = new QSqlQueryModel;
	userModel->setQuery(query);
	
	completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModel(userModel);
	completer->setCompletionColumn(0);
	completer->setWrapAround(true);
	
	editTreasurer->setEditable(true);
	editTreasurer->setCompleter(completer);
	editTreasurer->setModel(userModel);
	editTreasurer->setModelColumn(0);
}

void SectionEdit::showData(QModelIndex index) {
	i_index = index.row();
	resetSectionData();
}

void SectionEdit::resetSectionData() {
	Section s(listModel->index(i_index, 1).data().toString());
	labelName->setText( s.name() );
	editAmount->setValue( s.amount() );
	editDescription->setPlainText( s.description() );
	editBankAccountNumber->setText( s.account() );
	editAddress->setPlainText( s.address() );
	editFoundingDate->setDate( s.founded() );
	editParent->setCurrentIndex( editParent->findData( s.parent()->name() ) );
	
	QModelIndexList list = userModel->match(userModel->index(0, 1), Qt::DisplayRole, QString::number(s.treasurer()), 1, Qt::MatchExactly);
	if (list.size() > 0) {
		editTreasurer->setCurrentIndex( list[0].row() );
	} else {
		editTreasurer->setCurrentIndex(-1);
	}
}

void SectionEdit::saveSection() {
	Section s(listModel->index(i_index, 1).data().toString());
	s.setAmount(editAmount->value());
	s.setDescription(editDescription->toPlainText());
	s.setAccount(editBankAccountNumber->text());
	s.setAddress(editAddress->toPlainText());
	s.setFoundingDate(editFoundingDate->date());
	s.setParent( editParent->itemText(editParent->currentIndex()) );
	
	QModelIndexList list = userModel->match(userModel->index(0, 0), Qt::DisplayRole, editTreasurer->itemText(editTreasurer->currentIndex()), 1, Qt::MatchExactly);
	if (list.size() > 0) {
		QModelIndex idx = userModel->index(list[0].row(), 1);
		s.setTreasurer(idx.data().toInt());
	} else {
		s.setTreasurer(0);
	}
	s.save();
}
