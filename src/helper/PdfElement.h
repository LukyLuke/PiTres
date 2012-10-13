/*
	An Element which can be drawn onto a PDF - Element-Implementatins are below
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

#ifndef HELPER_PDFELEMENT_H
#define HELPER_PDFELEMENT_H

#include <QObject>
#include <QDomElement>
#include <QDomNode>
#include <QDomNamedNodeMap>
#include <QHash>
#include <QString>
#include <QPainter>
#include <QList>

enum PdfElementType { PdfUnknown=-1, PdfLine=0, PdfCircle=1, PdfArc=2, PdfEllipse=3, PdfRectangle=5, PdfPolygon=6, PdfText=7, PdfImage=8 };

class PdfElement {
public:
	PdfElement();
	PdfElement(const PdfElement &newPdfElement);
	virtual ~PdfElement();
	void paint(QPainter *painter);
	void setElement(QDomElement element);
	void setAttributes(const QDomNamedNodeMap attr, const QString cdata);
	void setType(PdfElementType type);
	void setTemplatePath(QString templatePath);
	void setVars(QHash<QString, QString> *variables);
	
	QHash<QString, QString> attributes() const { return _attributes; };
	PdfElementType type() const { return _type; };
	QList<PdfElement> nodes() const { return _nodes; };
	QString templatePath() const { return _templatePath; };
	
	static PdfElement fromElement(QDomElement element, QString fromElement);

protected:
	PdfElementType _type;
	QList<PdfElement> _nodes;
	QHash<QString, QString> _attributes;
	QHash<QString, QString> *_variables;
	QString _templatePath;
	PdfElement parseType(QString nodeName);
	void parse(QDomNode node);
	qreal toQReal(QString value);
	qreal offsetX;
	qreal offsetY;
};

class PdfElementLine : public PdfElement {
public:
	PdfElementLine();
	virtual ~PdfElementLine();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementCircle : public PdfElement {
public:
	PdfElementCircle();
	virtual ~PdfElementCircle();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementArc : public PdfElement {
public:
	PdfElementArc();
	virtual ~PdfElementArc();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementEllipse : public PdfElement {
public:
	PdfElementEllipse();
	virtual ~PdfElementEllipse();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementRectangle : public PdfElement {
public:
	PdfElementRectangle();
	virtual ~PdfElementRectangle();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementPolygon : public PdfElement {
public:
	PdfElementPolygon();
	virtual ~PdfElementPolygon();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementText : public PdfElement {
public:
	PdfElementText();
	virtual ~PdfElementText();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

class PdfElementImage : public PdfElement {
public:
	PdfElementImage();
	virtual ~PdfElementImage();
	void setType(PdfElementType type);
	void paint(QPainter *painter);
};

#endif // HELPER_PDFELEMENT_H