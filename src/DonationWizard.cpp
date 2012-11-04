/*
	The DonaitionWizard is for importing and adding Donations
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

#include "DonationWizard.h"

DonationWizard::DonationWizard(QWidget *parent) : QDialog(parent) {
	setupUi(this);
	
	connect(btnImportBack, SIGNAL(clicked()), this, SLOT(wizardBack()));
	connect(btnImportNext, SIGNAL(clicked()), this, SLOT(wizardNext()));
	connect(btnImportSave, SIGNAL(clicked()), this, SLOT(wizardNext()));
	
	connect(btnChooseDonationImport, SIGNAL(clicked()), this, SLOT(showFileDialog()));
	connect(actionReloadImportfile, SIGNAL(triggered()), this, SLOT(reloadImport()));
	
	Section::getSectionList(&sectionsList);
	
	enableWizardButtons();
}

DonationWizard::~DonationWizard() {}

void DonationWizard::wizardBack() {
	if (stackedImport->currentIndex() >= 0) {
		stackedImport->setCurrentIndex(stackedImport->currentIndex()-1);
	}
	enableWizardButtons();
}

void DonationWizard::wizardNext() {
	if (stackedImport->currentIndex() < (stackedImport->count()-1)) {
		stackedImport->setCurrentIndex(stackedImport->currentIndex()+1);
	}
	enableWizardButtons();
}

void DonationWizard::enableWizardButtons() {
	switch (stackedImport->currentIndex()) {
		case 0: // Choose File
			btnImportBack->setEnabled(false);
			btnImportNext->setEnabled(true);
			btnImportSave->setEnabled(false);
			break;
			
		case 1: // Show Data
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(true);
			btnImportSave->setEnabled(false);
			
			reloadImport();
			break;
			
		case 2: // Section-Mapping
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(true);
			btnImportSave->setEnabled(false);
			break;
			
		case 3: // Preview
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(false);
			btnImportSave->setEnabled(true);
			
			previewImport();
			break;
			
		case 4: // import/save
			btnImportBack->setEnabled(true);
			btnImportNext->setEnabled(false);
			btnImportSave->setEnabled(false);
			
			wizardImport();
			break;
	}
}

void DonationWizard::wizardPrepareRecord() {
	
}

void DonationWizard::reloadImport() {
	tablePreviewImport->clear();
	loadedSections.clear();
	
	QFile file(editImportFile->text());
	if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		return;
	}
	
	QStringList list;
	QByteArray line;
	QString val;
	bool firstLine(true);
	qint32 row = 0, col = 0, pos = 0;
	qint32 sectionColum = importSectionColumn->value() - 1;
	qint32 column = importColumnMaster->value() - 1;
	QRegExp match(importRegexMatch->text(), Qt::CaseInsensitive);
	QString repl(importRegexReplace->text());
	QString sep((importSeparator->text().size() > 0) ? importSeparator->text().at(0) : ';');
	QRegExp split(QString("(\"([^\"]*)\"%1?)|(([^%1]*)%1?)").arg(sep));
	
	while (!file.atEnd()) {
		if (firstLine) {
			firstLine = false;
			if (checkFirstLineHeader->checkState() == Qt::Checked) {
				line = file.readLine();
				continue;
			}
		}
		
		if (tablePreviewImport->rowCount() <= row) {
			tablePreviewImport->insertRow(tablePreviewImport->rowCount());
		}
		
		pos = 0;
		col = 0;
		line = file.readLine();
		while ( (line.size() > pos) && ((pos = split.indexIn(line, pos)) != -1) ) {
			val = split.cap(2);
			if (val.size() <= 0) {
				val = split.cap(4);
			}
			pos += split.matchedLength();
			
			if (col == column) {
				val.replace(match, repl);
			}
			if (sectionColum >= 0 && col == sectionColum && !loadedSections.contains(val)) {
				loadedSections.insert(val, "");
			}
			
			if (!row && tablePreviewImport->columnCount() <= col) {
				tablePreviewImport->insertColumn(tablePreviewImport->columnCount());
			}
			
			tablePreviewImport->setItem(row, col, new QTableWidgetItem(val));
			col++;
		}
		row++;
	}
	createSectionMapping();
}

void DonationWizard::createSectionMapping() {
	QFormLayout *layout = (QFormLayout *) importMapSections->layout();
	QLayoutItem *item;
	while (layout->count() && (item = layout->takeAt(0)) != 0) {
		delete (item->widget());
		delete item;
	}
	
	int i, idx;
	QMap<QString, QString>::iterator it;
	for (it = loadedSections.begin(); it != loadedSections.end(); it++) {
		QComboBox *b = new QComboBox;
		idx = -1;
		for (i = 0; i < sectionsList.size(); i++) {
			b->addItem(sectionsList[i], sectionsList[i]);
			if ( (idx < 0) && (it.key().indexOf(sectionsList[i], 0, Qt::CaseInsensitive) >= 0) ) {
				idx = i;
			}
		}
		b->setCurrentIndex(idx < 0 ? 0 : idx);
		layout->addRow(it.key(), b);
	}
}

void DonationWizard::previewImport() {
	tableWidget->clear();
	if (tableWidget->columnCount() < 5) {
		tableWidget->setColumnCount(5);
	}
	tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem());
	tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem());
	tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem());
	tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem());
	tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem());
	tableWidget->horizontalHeaderItem(0)->setText(QApplication::translate("DonationImportForm", "Member", 0, QApplication::UnicodeUTF8));
	tableWidget->horizontalHeaderItem(1)->setText(QApplication::translate("DonationImportForm", "Name, Surname", 0, QApplication::UnicodeUTF8));
	tableWidget->horizontalHeaderItem(2)->setText(QApplication::translate("DonationImportForm", "UID", 0, QApplication::UnicodeUTF8));
	tableWidget->horizontalHeaderItem(3)->setText(QApplication::translate("DonationImportForm", "Amount", 0, QApplication::UnicodeUTF8));
	tableWidget->horizontalHeaderItem(4)->setText(QApplication::translate("DonationImportForm", "Section", 0, QApplication::UnicodeUTF8));
	
	QLocale locale;
	QHash<QString, QString> sectionMapping;
	qint32 pos = 0;
	QFormLayout *layout = (QFormLayout *) importMapSections->layout();
	QLabel *label;
	QComboBox *box;
	while (pos < layout->rowCount()) {
		label = (QLabel *) layout->itemAt(pos, QFormLayout::LabelRole)->widget();
		box = (QComboBox *) layout->itemAt(pos, QFormLayout::FieldRole)->widget();
		sectionMapping.insert(label->text(), box->itemData(box->currentIndex()).toString());
		pos++;
	}
	
	PPSPerson person;
	qint32 sectionColum = importSectionColumn->value() - 1;
	qint32 column = importColumnMaster->value() - 1;
	qint32 amountColum = importAmountColumn->value() - 1;
	qint32 dbField = importColumnMemberfields->currentIndex();
	QString uid, amount, section;
	
	for (int row = 0; row < tablePreviewImport->rowCount(); row++) {
		uid = tablePreviewImport->item(row, column)->text();
		amount = tablePreviewImport->item(row, amountColum)->text();
		section = tablePreviewImport->item(row, sectionColum)->text();
		bool found(false);
		
		switch (dbField) {
			case 1:
				found = person.loadByPersonsFields(uid);
				break;
			case 2:
				found = person.loadByTelephone(uid);
				break;
			case 3:
				found = person.loadByEmail(uid);
				break;
			default:
				found = person.loadByAnyField(uid);
		}
		
		tableWidget->insertRow(row);
		if (found) {
			tableWidget->setItem(row, 0, new QTableWidgetItem(person.uid()));
			tableWidget->setItem(row, 1, new QTableWidgetItem(person.givenName() + " " + person.familyName()));
		} else {
			tableWidget->setItem(row, 0, new QTableWidgetItem(""));
			tableWidget->setItem(row, 1, new QTableWidgetItem(""));
		}
		tableWidget->setItem(row, 2, new QTableWidgetItem(uid));
		tableWidget->setItem(row, 3, new QTableWidgetItem(locale.toString(amount.toFloat())));
		tableWidget->setItem(row, 4, new QTableWidgetItem(sectionMapping.value(section)));
	}
}

void DonationWizard::wizardImport() {
	
}

void DonationWizard::showFileDialog() {
	editImportFile->setText(
		QFileDialog::getOpenFileName(this, tr("Load Donations-File"), "", tr("Comma seperated File (*.csv);;Quicken Interchange Format (*.qif)"))
	);
}

