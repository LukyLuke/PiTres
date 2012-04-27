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
	void paint(QPainter painter);
	void setElement(QDomElement element);
	void setAttributes(const QDomNamedNodeMap attr, const QString cdata);
	void setType(PdfElementType type);
	
	QHash<QString, QString> attributes() const { return _attributes; };
	PdfElementType type() const { return _type; };
	QList<PdfElement> nodes() const { return _nodes; };
	
	static PdfElement fromElement(QDomElement element);

protected:
	PdfElementType _type;
	QList<PdfElement> _nodes;
	QHash<QString, QString> _attributes;
	PdfElement parseType(QString nodeName);
	void parse(QDomNode node);
};

class PdfElementLine : public PdfElement {
public:
	PdfElementLine();
	virtual ~PdfElementLine();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementCircle : public PdfElement {
public:
	PdfElementCircle();
	virtual ~PdfElementCircle();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementArc : public PdfElement {
public:
	PdfElementArc();
	virtual ~PdfElementArc();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementEllipse : public PdfElement {
public:
	PdfElementEllipse();
	virtual ~PdfElementEllipse();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementRectangle : public PdfElement {
public:
	PdfElementRectangle();
	virtual ~PdfElementRectangle();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementPolygon : public PdfElement {
public:
	PdfElementPolygon();
	virtual ~PdfElementPolygon();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementText : public PdfElement {
public:
	PdfElementText();
	virtual ~PdfElementText();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

class PdfElementImage : public PdfElement {
public:
	PdfElementImage();
	virtual ~PdfElementImage();
	void setType(PdfElementType type);
	void paint(QPainter painter);
};

#endif // HELPER_PDFELEMENT_H