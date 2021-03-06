/*
	A PDF-Generator based on simple XML Documents
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

#include "XmlPdf.h"

#include <QDebug>

XmlPdf::XmlPdf(QObject * /*parent*/) {
	doc = QDomDocument("template");
	paperSize = QPrinter::A4;
	currentPage = 1;
}

XmlPdf::~XmlPdf() {
	clearEntries();
}

void XmlPdf::loadFonts() {
	QFontDatabase::addApplicationFont(QString::fromUtf8(":/resources/ocrb.ttf"));
}

void XmlPdf::loadTemplate(QString file) {
	QFile tpl(file);
	if (!tpl.open(QIODevice::ReadOnly)) {
		qDebug() << "Unable to load the File: " + file;
		return;
	}
	if (!doc.setContent(&tpl)) {
		tpl.close();
		qDebug() << "Failed to load the File as XML...";
		return;
	}
	tpl.close();
	
	QFileInfo info(file);
	templatePath = info.canonicalPath();
	loadFonts();
	elements.clear();
	
	QDomNodeList tl = doc.elementsByTagName("template");
	if (tl.size() > 0 && tl.at(0).isElement()) {
		QDomElement t = tl.at(0).toElement();
		paperSize = QPrinter::A4;
		if (t.hasAttribute("size")) {
			QString size = t.attribute("size").toUpper();
			if (size == "A4") {
				paperSize = QPrinter::A4;
			} else if (size == "A5") {
				paperSize = QPrinter::A5;
			} else if (size == "A6") {
				paperSize = QPrinter::A6;
			} else if (size == "LETTER") {
				paperSize = QPrinter::Letter;
			}
		}
		
		paperOrientation = QPrinter::Portrait;
		if (t.hasAttribute("orientation")) {
			QString orient = t.attribute("orientation").toLower();
			if (orient == "landscape") {
				paperOrientation = QPrinter::Landscape;
			}
		}
		
		QDomNodeList nl = t.childNodes();
		for (int i = 0; i < nl.size(); i++) {
			QDomNode n = nl.item(i);
			if (n.isElement() && n.nodeName().toLower() == "g") {
				QDomElement elem = n.toElement();
				if (elem.hasAttribute("part") && !elem.attribute("part").isEmpty()) {
					elements.insert(elem.attribute("part").toLower(), PdfElement::fromElement(elem, templatePath));
				}
			}
		}
	}
}

void XmlPdf::setVar(QString name, QString value) {
	variables.insert(name, value);
}

bool XmlPdf::print(QString file) {
	QPrinter printer(QPrinter::HighResolution);
	printer.setPaperSize(paperSize);
	printer.setOrientation(paperOrientation);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setPageMargins(8.53, 8.53, 3.53, 3.53, QPrinter::Millimeter);
	
	if (file.isEmpty()) {
		QPrintDialog *dialog = new QPrintDialog(&printer);
		dialog->setWindowTitle(QObject::tr("Print Document"));
		if (dialog->exec() != QDialog::Accepted) {
			qDebug() << "Printing aborted and no Filename given, canceled printing...";
			return false;
		}
	} else {
		printer.setOutputFileName(file);
	}
	
	QPainter *painter = new QPainter();
	if (!painter->begin(&printer)) {
		qDebug() << QString("The File is not writable:\n%1").arg(file);
		return false;
	}
	
	addDynamics(painter, &printer);
	addStatics(painter);
	painter->end();
	return true;
}

void XmlPdf::addDynamics(QPainter *painter, QPrinter *printer) {
	qreal bottom, nextBottom;
	qreal paperHeight = printer->pageRect(QPrinter::DevicePixel).height();
	QHash<QString, PdfElement>::const_iterator it;
	QHash<QString, QList<XmlPdfEntry*> >::const_iterator rit;

	// Set the pagenumber and calculate number of pages
	qint32 num_pages = 1;
	for (rit = repeatingEntries.constBegin(); rit != repeatingEntries.constEnd(); rit++) {
		if (!elements.contains(rit.key())) {
			continue;
		}
		
		QList<XmlPdfEntry*> list = rit.value();
		it = elements.find(rit.key());
		if (it != elements.constEnd()) {
			bottom = 0;
			nextBottom = 0;
			for (int i = 0; i < list.size(); i++) {
				if (nextBottom >= paperHeight) {
					num_pages++;
					bottom = 0;
				}
				PdfElement elem = it.value();
				elem.setTop(bottom);
				bottom = elem.bottom(i+1, list.size());
				if (bottom >= 0) {
					nextBottom = bottom + (bottom - elem.top()) + elem.bottomSpace();
				}
			}
		}
	}
	variables["current_page"] = QString::number(currentPage);
	variables["num_pages"] = QString::number(num_pages);
	
	// Same as above, but this time we print the PDF
	for (rit = repeatingEntries.constBegin(); rit != repeatingEntries.constEnd(); rit++) {
		if (!elements.contains(rit.key())) {
			continue;
		}
		
		QList<XmlPdfEntry*> list = rit.value();
		it = elements.find(rit.key());
		if (it != elements.constEnd()) {
			bottom = 0;
			nextBottom = 0;
			for (int i = 0; i < list.size(); i++) {
				if (nextBottom >= paperHeight) {
					addStatics(painter, currentPage, num_pages);
					printer->newPage();
					bottom = 0;
					currentPage++;
					variables["current_page"] = QString::number(currentPage);
				}
				
				PdfElement elem = it.value();
				if (elem.checkShow(i+1, list.size())) {
					elem.setVars(&variables, (list.at(i))->getVariables());
					elem.setTop(bottom);
					bottom = elem.paint(painter, i+1, list.size());
					nextBottom = bottom + (bottom - elem.top()) + elem.bottomSpace();
				}
			}
		}
	}
}

void XmlPdf::addStatics(QPainter *painter, int row, int max) {
	QHash<QString, PdfElement>::const_iterator it;
	for (it = elements.constBegin(); it != elements.constEnd(); it++) {
		if (repeatingEntries.contains(it.key())) {
			continue;
		}
		PdfElement elem = it.value();
		elem.setVars(&variables);
		if (row == 0 && max == 0) {
			elem.paint(painter, variables["num_pages"].toInt(), variables["num_pages"].toInt());
		} else {
			elem.paint(painter, row, max);
		}
	}
}

bool XmlPdf::send(QString email) {
	QSettings settings;
	
	// Create a tmp file form the PDF
	char fname [L_tmpnam];
	tmpnam(fname);
	QString fileName(fname);
	print(fileName);
	
	Smtp mail(settings.value("smtp/host", "localhost").toString(), settings.value("smtp/port", 587).toInt());
	if (settings.value("smtp/authentication", "login").toString() == "login") {
		mail.authLogin(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	} else if (settings.value("smtp/authentication", "login").toString() == "plain") {
		mail.authPlain(settings.value("smtp/username", "").toString(), settings.value("smtp/password", "").toString());
	}
	
	// Attach the PDF and remove the tmp file
	mail.attach(fileName, "Invoice.pdf");
	remove(fname);
	
	// Get the EMail-Message from the Template
	QString subject;
	QString textMessage;
	QString htmlMessage;
	QString type;
	QDomNodeList tl = doc.elementsByTagName("email");
	for (int j = 0; j < tl.size(); j++) {
		if (tl.at(j).isElement()) {
			QDomElement t = tl.at(j).toElement();
			if (t.hasAttribute("type")) {
				type = t.attribute("type").toLower();
				
				QDomNodeList nl = t.childNodes();
				for (int i = 0; i < nl.size(); i++) {
					QDomNode n = nl.item(i);
					
					if (n.isElement() && n.nodeName().toLower() == "subject") {
						subject = n.toElement().text();
					} else if (n.isElement() && n.nodeName().toLower() == "content") {
						if (type == "text") {
							textMessage = n.toElement().text();
						} else if (type == "html") {
							htmlMessage = n.toElement().text();
						}
					}
				}
				
			}
		}
	}
	
	// Replace variables in Email-Texts
	QHash<QString, QString>::const_iterator it;
	for (it = variables.constBegin(); it != variables.constEnd(); it++) {
		subject.replace(QString("{%1}").arg(it.key()), it.value(), Qt::CaseInsensitive);
		textMessage.replace(QString("{%1}").arg(it.key()), it.value(), Qt::CaseInsensitive);
		htmlMessage.replace(QString("{%1}").arg(it.key()), it.value(), Qt::CaseInsensitive);
	}
	mail.setTextMessage(textMessage);
	mail.setHtmlMessage(htmlMessage);
	
	// Send the Mail
	return mail.send(settings.value("smtp/from", "me@noreply.dom").toString(), email, subject);
}

void XmlPdf::clearEntries() {
	QHash<QString, QList<XmlPdfEntry*> >::iterator it;
	for (it = repeatingEntries.begin(); it != repeatingEntries.end(); it++) {
		while (!(it.value()).isEmpty()) {
			delete (it.value()).takeFirst();
		}
	}
	repeatingEntries.clear();
}

XmlPdfEntry* XmlPdf::addEntry(QString name) {
	if (!repeatingEntries.contains(name)) {
		QList<XmlPdfEntry *> list;
		repeatingEntries.insert(name, list);
	}
	repeatingEntries[name].append(new XmlPdfEntry);
	return repeatingEntries[name].last();
}

XmlPdfEntry* XmlPdf::getEntry(QString name, int num) {
	if (num < 0 || !repeatingEntries.contains(name) || num >= repeatingEntries[name].size()) {
		return 0;
	}
	return repeatingEntries[name].at(num);
}


/**
 * The XmlPdfEntry Class is for repeating sections inside a XmlPdf
 */
XmlPdfEntry::XmlPdfEntry(QObject * /*parent*/) { }
XmlPdfEntry::~XmlPdfEntry() { }

void XmlPdfEntry::setVar(QString key, QString value) {
	variables.insert(key, value);
}

void XmlPdfEntry::setVar(QString key, qint32 value) {
	variables.insert(key, QString::number(value));
}

void XmlPdfEntry::setVar(QString key, float value) {
	QLocale locale;
	variables.insert(key, locale.toString(value, 'f', 2));
}

QString XmlPdfEntry::getValue(QString key) {
	QHash<QString, QString>::const_iterator it = variables.find(key);
	if (it != variables.end()) {
		return it.value();
	}
	return QString("");
}

QHash<QString, QString> *XmlPdfEntry::getVariables() {
	return &variables;
}

