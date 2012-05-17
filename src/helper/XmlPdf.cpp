
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
#include <QPrintDialog>
#include <QMetaEnum>
#include <QDebug>

XmlPdf::XmlPdf(QObject *parent) {
	doc = QDomDocument("template");
	paperSize = QPrinter::A4;
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
	
	QDomNodeList tl = doc.elementsByTagName("template");
	if (tl.size() > 0 && tl.at(0).isElement()) {
		QDomElement t = tl.at(0).toElement();
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
			} else {
				paperSize = QPrinter::A4;
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
	QPrinter printer(QPrinter::PrinterResolution);
	printer.setPaperSize(paperSize);
	printer.setOutputFormat(QPrinter::PdfFormat);
	
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
	qDebug() << "Sending EMail with PDF not implemented currently :(";
	return false;
}
