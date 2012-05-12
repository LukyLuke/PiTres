
#include "XmlPdf.h"
#include "PdfElement.h"

#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QFontDatabase>
#include <QPainter>
#include <QPrinter>
#include <QDebug>

XmlPdf::XmlPdf(QObject *parent) {
	doc = QDomDocument("template");
}

XmlPdf::~XmlPdf() { }

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
	QDomNodeList nl = doc.elementsByTagName("g");
	for (int i = 0; i < nl.size(); i++) {
		QDomNode n = nl.item(i);
		if (n.isElement()) {
			QDomElement elem = n.toElement();
			if (elem.hasAttribute("part") && !elem.attribute("part").isEmpty()) {
				elements.insert(elem.attribute("part").toLower(), PdfElement::fromElement(elem, templatePath));
			}
		}
	}
}

void XmlPdf::setVar(QString name, QString value) {
	variables.insert(name, value);
}


bool XmlPdf::print(QString file) {
	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(file);
	
	QPainter *painter = new QPainter();
	if (!painter->begin(&printer)) {
		qDebug() << QString("The File is not writable:\n%1").arg(file);
		return false;
	}
	
	QHash<QString, PdfElement>::const_iterator it = elements.constBegin();
	while (it != elements.constEnd()) {
		PdfElement elem = it.value();
		elem.setVars(&variables);
		elem.paint(painter);
		it++;
	}
	painter->end();
	return true;
}

bool XmlPdf::send(QString email) {
	
}
