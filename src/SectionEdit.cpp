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
	connect(btnReassign, SIGNAL(clicked()), this, SLOT(reassignInvoices()));
	
	QSqlQuery query(db);
	query.exec("SELECT UPPER(name) || ', ' || description || CASE WHEN parent IS NULL THEN '' ELSE ' (' || parent || ')' END AS label, name FROM pps_sections ORDER BY parent,name ASC;");
	listModel = new QSqlQueryModel;
	listModel->setQuery(query);
	sectionList->setModel(listModel);
	sectionList->setModelColumn(0);
	
	// Reassignment table
	reassignModel = new QSqlQueryModel(tableInvoices);
	setReassignQuery();
	reassignModel->setHeaderData(0, Qt::Horizontal, tr("Section"));
	reassignModel->setHeaderData(1, Qt::Horizontal, tr("Member"));
	reassignModel->setHeaderData(2, Qt::Horizontal, tr("Name"));
	reassignModel->setHeaderData(3, Qt::Horizontal, tr("Paid date"));
	reassignModel->setHeaderData(4, Qt::Horizontal, tr("Issued"));
	reassignModel->setHeaderData(5, Qt::Horizontal, tr("Reference"));
	tableInvoices->setModel(reassignModel);
	
	initComboBoxes();
}

SectionEdit::~SectionEdit() {
	delete listModel;
	delete userModel;
	delete tableInvoices;
}

void SectionEdit::setReassignQuery() {
	Section s(listModel->index(i_index, 1).data().toString());
	QSqlQuery query(db);
	query.prepare("SELECT pps_invoice.for_section, ldap_persons.section, pps_invoice.address_name, pps_invoice.paid_date, pps_invoice.issue_date, pps_invoice.reference"
	              " FROM pps_invoice LEFT JOIN ldap_persons ON (pps_invoice.member_uid = ldap_persons.uid)"
	              " WHERE ldap_persons.section = :section AND pps_invoice.for_section != ldap_persons.section"
	              " AND (pps_invoice.paid_date = '' OR pps_invoice.paid_date > :founded) AND pps_invoice.issue_date >= :year;");
	query.bindValue(":section", s.name());
	query.bindValue(":founded", s.founded());
	query.bindValue(":year", s.founded().toString("yyyy-01-01"));
	query.exec();
	reassignModel->setQuery(query);
}

void SectionEdit::reassignInvoices() {
	if (reassignModel->query().first()) {
		QSqlQuery query(db);
		query.prepare("UPDATE pps_invoice SET for_section=:section WHERE reference=:reference AND for_section=:oldsection;");
		do {
			query.bindValue(":section", reassignModel->query().value(1));
			query.bindValue(":reference", reassignModel->query().value(5));
			query.bindValue(":oldsection", reassignModel->query().value(0));
			query.exec();
		} while (reassignModel->query().next());
	}
}

void SectionEdit::initComboBoxes() {
	// The Parent-Combobox
	QSqlQuery query(db);
	query.exec("SELECT UPPER(name) || ', ' || description || CASE WHEN parent IS NULL THEN '' ELSE ' (' || parent || ')' END AS label, name FROM pps_sections "
	           "ORDER BY parent,name ASC;");
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
	setReassignQuery();
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
	s.setParent( editParent->itemData(editParent->currentIndex()).toString() );
	
	QModelIndexList list = userModel->match(userModel->index(0, 0), Qt::DisplayRole, editTreasurer->itemText(editTreasurer->currentIndex()), 1, Qt::MatchExactly);
	if (list.size() > 0) {
		QModelIndex idx = userModel->index(list[0].row(), 1);
		s.setTreasurer(idx.data().toInt());
	} else {
		s.setTreasurer(0);
	}
	s.save();
}
