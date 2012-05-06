
#include "XmlPdf.h"
#include "PdfElement.h"

#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QIODevice>
#include <QFontDatabase>
#include <QHashIterator>
#include <QPainter>
#include <QPrinter>
#include <QDebug>

XmlPdf::XmlPdf(QObject *parent) {
	QFontDatabase::addApplicationFont("aller.ttf");
	QFontDatabase::addApplicationFont("b0.ttf");
	QFontDatabase::addApplicationFont("allerlight.ttf");
}

XmlPdf::~XmlPdf() {
}

void XmlPdf::loadTemplate(QString file) {
	QDomDocument doc("template");
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
	
	elements.clear();
	QDomNodeList nl = doc.elementsByTagName("g");
	for (int i = 0; i < nl.size(); i++) {
		QDomNode n = nl.item(i);
		if (n.isElement()) {
			QDomElement elem = n.toElement();
			if (elem.hasAttribute("part") && !elem.attribute("part").isEmpty()) {
				elements.insert(elem.attribute("part").toLower(), PdfElement::fromElement(elem));
			}
		}
	}
}

bool XmlPdf::print(QString file) {
	QHashIterator<QString, PdfElement> it(elements);
	QPrinter printer;
	printer.setPaperSize(QPrinter::A4);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(file);
	
	QPainter *painter = new QPainter();
	if (!painter->begin(&printer)) {
		qDebug() << QString("The File is not writable:\n%1").arg(file);
		return false;
	}
	
	while (it.hasNext()) {
		it.next();
		PdfElement elem = it.value();
		elem.paint(painter);
	}
	painter->end();
	return true;
}
