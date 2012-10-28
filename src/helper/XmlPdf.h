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

#ifndef HELPER_XMLPDF_H
#define HELPER_XMLPDF_H

#include "PdfElement.h"
#include "Smtp.h"

#include <QObject>
#include <QHash>
#include <QList>
#include <QPrinter>
#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QFontDatabase>
#include <QPainter>
#include <QPrintDialog>
#include <QMetaEnum>
#include <QSettings>

struct StaticQtMetaObject : public QObject {
	static inline const QMetaObject& get() {
		return staticQtMetaObject;
	}
};

class XmlPdfEntry; // See below XmlPdf for declaration

class XmlPdf {
public:
	XmlPdf(QObject *parent = 0);
	virtual ~XmlPdf();
	void loadTemplate(QString file);
	void setVar(QString name, QString value);
	XmlPdfEntry* addEntry(QString name);
	XmlPdfEntry* getEntry(QString name, int num);
	void clearEntries();
	bool print(QString file);
	bool send(QString email);

private:
	QHash<QString, PdfElement> elements;
	QHash<QString, QString> variables;
	QHash<QString, QList<XmlPdfEntry *> > repeatingEntries;
	QString templatePath;
	QPrinter::PageSize paperSize;
	QPrinter::Orientation paperOrientation;
	QDomDocument doc;
	qint32 currentPage;
	
	void loadFonts();
	void addStatics(QPainter *painter);
	void addDynamics(QPainter *painter, QPrinter *printer);
};

/**
 * The XmlPdfEntry Class is for repeating sections inside a XmlPdf
 */
class XmlPdfEntry {
public:
	XmlPdfEntry(QObject *parent = 0);
	virtual ~XmlPdfEntry();
	void setVar(QString key, QString value);
	void setVar(QString key, qint32 value);
	void setVar(QString key, float value);
	QString getValue(QString key);
	QHash<QString, QString> *getVariables();
	
private:
	QHash<QString, QString> variables;
};

#endif // HELPER_XMLPDF_H