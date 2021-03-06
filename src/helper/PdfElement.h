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
#include <QtSvg/QtSvg>
#include <QtSvg/QSvgRenderer>
#include <QList>

enum PdfElementType { PdfUnknown=-1, PdfLine=0, PdfCircle=1, PdfArc=2, PdfEllipse=3, PdfRectangle=5, PdfPolygon=6, PdfText=7, PdfImage=8 };

class PdfElement {
public:
	PdfElement();
	PdfElement(const PdfElement &newPdfElement);
	virtual ~PdfElement();
	qreal paint(QPainter *painter, const qint32 row = 0, const qint32 maxrows = 0);
	qreal bottom(const qint32 row = 0, const qint32 maxrows = 0);
	void setElement(QDomElement element);
	void setAttributes(const QDomNamedNodeMap attr, const QString cdata);
	void setType(PdfElementType type);
	void setTemplatePath(QString templatePath);
	void setVars(QHash<QString, QString> *variables);
	void setVars(QHash<QString, QString> *variables, QHash<QString, QString> *repeating);
	void setTop(qreal top);

	QHash<QString, QString> attributes() const { return _attributes; };
	PdfElementType type() const { return _type; };
	QList<PdfElement> nodes() const { return _nodes; };
	QString templatePath() const { return _templatePath; };
	qreal top() const { return _offsetY; };
	qreal bottomSpace() const { return _bottomSpace; };
	bool checkShow(const qint32 row, const qint32 maxrows) const;
	
	static PdfElement fromElement(QDomElement element, QString fromElement);
	static bool lengthDescending(const QString &a, const QString &b) { return a.length() > b.length(); };

protected:
	PdfElementType _type;
	QList<PdfElement> _nodes;
	QHash<QString, QString> _attributes;
	QHash<QString, QString> *_variables;
	QHash<QString, QString> *_repeating;
	QString _templatePath;
	qreal _offset;
	qreal _bottomSpace;
	qreal _top;
	qreal _offsetY;
	bool _onlyOnLast;
	bool _onlyOnFirst;
	qint32 _onlyOnNth;

	PdfElement parseType(QString nodeName);
	void parse(QDomNode node);
	qreal toQReal(QString value);

	struct expression {
		bool (*expr)(const QString*, const QString*);
		QString successfull;
		QString otherwise;
		QString value;
		QString variable;
		bool valid() { return !variable.isEmpty() && expr != NULL; };
	};
	static bool expr_gt(const QString *a, const QString *b) { return a->toFloat() >  b->toFloat(); }
	static bool expr_lt(const QString *a, const QString *b) { return a->toFloat() <  b->toFloat(); }
	static bool expr_ge(const QString *a, const QString *b) { return a->toFloat() >= b->toFloat(); }
	static bool expr_le(const QString *a, const QString *b) { return a->toFloat() <= b->toFloat(); }
	static bool expr_eq(const QString *a, const QString *b) { return a->toLower() == b->toLower(); }
	static bool expr_ne(const QString *a, const QString *b) { return a->toLower() != b->toLower(); }
	static bool expr_contains(const QString *a, const QString *b) { return a->toLower().contains(b->toLower()); }

	QHash<QString, expression> _expressions;
	void extractExpressions(QString *cdata);
	void evalExpression(QString *cdata, const QString &var, const QString &val);
};

class PdfElementLine : public PdfElement {
public:
	PdfElementLine();
	virtual ~PdfElementLine();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementCircle : public PdfElement {
public:
	PdfElementCircle();
	virtual ~PdfElementCircle();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementArc : public PdfElement {
public:
	PdfElementArc();
	virtual ~PdfElementArc();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementEllipse : public PdfElement {
public:
	PdfElementEllipse();
	virtual ~PdfElementEllipse();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementRectangle : public PdfElement {
public:
	PdfElementRectangle();
	virtual ~PdfElementRectangle();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementPolygon : public PdfElement {
public:
	PdfElementPolygon();
	virtual ~PdfElementPolygon();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementText : public PdfElement {
public:
	PdfElementText();
	virtual ~PdfElementText();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
};

class PdfElementImage : public PdfElement {
public:
	PdfElementImage();
	virtual ~PdfElementImage();
	void setType(PdfElementType type);
	qreal paint(QPainter *painter);
	qreal bottom();
	QList<QString> replaceVariables(QString file);
	void showDefaultImage(QPainter *painter, QRectF box);
};

#endif // HELPER_PDFELEMENT_H