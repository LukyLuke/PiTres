#ifndef HELPER_XMLPDF_H
#define HELPER_XMLPDF_H

#include "PdfElement.h"

#include <QObject>
#include <QHash>
#include <QPrinter>
#include <QString>

struct StaticQtMetaObject : public QObject {
	static inline const QMetaObject& get() {
		return staticQtMetaObject;
	}
};

class XmlPdf {
public:
	XmlPdf(QObject *parent = 0);
	virtual ~XmlPdf();
	void loadTemplate(QString file);
	void setVar(QString name, QString value);
	bool print(QString file);
	bool send(QString email);

private:
	QHash<QString, PdfElement> elements;
	QHash<QString, QString> variables;
	QString templatePath;
	QPrinter::PageSize paperSize;
	QPrinter::Orientation paperOrientation;
	QDomDocument doc;
	
	void loadFonts();
};

#endif // HELPER_XMLPDF_H