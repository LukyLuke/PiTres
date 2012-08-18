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