
#include "XmlPdf.h"
#include "PdfElement.h"

#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QIODevice>
#include <QFontDatabase>

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
		return;
	}
	if (!doc.setContent(&tpl)) {
		tpl.close();
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
