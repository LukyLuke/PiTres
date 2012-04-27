#ifndef HELPER_XMLPDF_H
#define HELPER_XMLPDF_H

#include "PdfElement.h"

#include <QObject>
#include <QHash>
#include <QString>

class XmlPdf {
public:
	XmlPdf(QObject *parent = 0);
	virtual ~XmlPdf();
	void loadTemplate(QString file);

private:
	QHash<QString, PdfElement> elements;
};

#endif // HELPER_XMLPDF_H