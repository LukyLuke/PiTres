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

#include "PdfElement.h"

#include <QString>
#include <QStringList>
#include <QHashIterator>
#include <QHash>
#include <QDomAttr>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QPolygonF>
#include <QFont>
#include <QImage>
#include <QApplication>
#include <QDebug>

PdfElement::PdfElement() {
	_offset = 0;
	_top = 0;
}

PdfElement::PdfElement(const PdfElement &newPdfElement) {
	_type = newPdfElement.type();
	_attributes = newPdfElement.attributes();
	_nodes = newPdfElement.nodes();
	_templatePath = newPdfElement.templatePath();
	_offset = newPdfElement._offset;
	_top = newPdfElement._top;
	_bottomSpace = newPdfElement._bottomSpace;
	_offsetY = newPdfElement._offsetY;
}

PdfElement::~PdfElement() {}

qreal PdfElement::paint(QPainter *painter) {
	PdfElementLine *_line;
	PdfElementCircle *_circle;
	PdfElementArc *_arc;
	PdfElementEllipse *_ellipse;
	PdfElementRectangle *_rectangle;
	PdfElementPolygon *_polygon;
	PdfElementText *_text;
	PdfElementImage *_image;
	qreal back = 0, bottom = 0;
	
	for (int i = 0; i < _nodes.size(); i++) {
		PdfElement e = _nodes.at(i);
		e.setVars(_variables, _repeating);
		e.setTop(_offsetY);
		switch (e.type()) {
			case PdfLine:
				_line = (PdfElementLine *)&e;
				back = _line->paint(painter);
				break;
			case PdfCircle:
				_circle = (PdfElementCircle *)&e;
				back = _circle->paint(painter);
				break;
			case PdfArc:
				_arc = (PdfElementArc *)&e;
				back = _arc->paint(painter);
				break;
			case PdfEllipse:
				_ellipse = (PdfElementEllipse *)&e;
				back = _ellipse->paint(painter);
				break;
			case PdfRectangle:
				_rectangle = (PdfElementRectangle *)&e;
				back = _rectangle->paint(painter);
				break;
			case PdfPolygon:
				_polygon = (PdfElementPolygon *)&e;
				back = _polygon->paint(painter);
				break;
			case PdfText:
				_text = (PdfElementText *)&e;
				back = _text->paint(painter);
				break;
			case PdfImage:
				_image = (PdfElementImage *)&e;
				back = _image->paint(painter);
				break;
		}
		bottom = back > bottom ? back : bottom;
	}
	return bottom + _offset;
}

PdfElement PdfElement::fromElement(QDomElement element, QString templatePath) {
	PdfElement e;
	e.setTemplatePath(templatePath);
	e.setElement(element);
	return e;
}

void PdfElement::setElement(QDomElement element) {
	_nodes.clear();
	_offset = element.hasAttribute("offset") ? toQReal(element.attribute("offset")) : 0;
	_top = element.hasAttribute("top") ? toQReal(element.attribute("top")) : 0;
	_bottomSpace = element.hasAttribute("bottom") ? toQReal(element.attribute("bottom")) : 0;
	
	for (QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling()) {
		parse(n);
	}
}

void PdfElement::setTemplatePath(QString templatePath) {
	_templatePath = templatePath;
}

void PdfElement::setType(PdfElementType pdfType) {
	_type = pdfType;
}

void PdfElement::setVars(QHash<QString, QString> *variables) {
	_variables = variables;
	_repeating = 0;
}

void PdfElement::setVars(QHash<QString, QString> *variables, QHash<QString, QString> *repeating) {
	_variables = variables;
	_repeating = repeating;
}

void PdfElement::setTop(qreal top) {
	if (top > _top) {
		_offsetY = top;
	} else {
		_offsetY = _top;
	}
}

void PdfElement::parse(QDomNode n) {
	if (!n.isNull() && n.isElement()) {
		QDomElement e = n.toElement();
		QString name = n.nodeName();
		PdfElement elem = parseType(name);
		if (_type != PdfUnknown) {
			elem.setType(_type);
			elem.setTop(0);
			elem.setTemplatePath(_templatePath);
			elem.setAttributes(e.attributes(), e.text());
			_nodes.push_back(elem);
		}
	}
}

PdfElement PdfElement::parseType(QString name) {
	name.toLower();
	if (name == "line") {
		_type = PdfLine;
		return PdfElementLine();
	} else if (name == "circle") {
		_type = PdfCircle;
		return PdfElementCircle();
	} else if (name == "arc") {
		_type = PdfArc;
		return PdfElementArc();
	} else if (name == "ellipse") {
		_type = PdfEllipse;
		return PdfElementEllipse();
	} else if (name == "rect") {
		_type = PdfRectangle;
		return PdfElementRectangle();
	} else if (name == "polygon") {
		_type = PdfPolygon;
		return PdfElementPolygon();
	} else if (name == "text") {
		_type = PdfText;
		return PdfElementText();
	} else if ((name == "img") || (name == "image")) {
		_type = PdfImage;
		return PdfElementImage();
	}
	_type = PdfUnknown;
	return PdfElement();
}

void PdfElement::setAttributes(const QDomNamedNodeMap attr, const QString cdata) {
	_attributes.clear();
	if (cdata != NULL) {
		_attributes.insert("cdata", cdata);
	}
	for (uint i = 0; i < attr.length(); i++) {
		QDomAttr a = attr.item(i).toAttr();
		if (!a.isNull()) {
			_attributes.insert(a.name().toLower(), a.value());
		}
	}
}

qreal PdfElement::toQReal(QString value) {
	qreal val = value.toFloat();
	return val * 12.5;
}

// Line
PdfElementLine::PdfElementLine() : PdfElement() {}
PdfElementLine::~PdfElementLine() {}
qreal PdfElementLine::paint(QPainter *painter) {
	qreal width = _attributes.value("stroke", "2").toFloat();
	QString strokeColor = _attributes.value("strokecolor", "black");
	qreal x1 = toQReal(_attributes.value("x1", "0"));
	qreal y1 = toQReal(_attributes.value("y1", "0")) + _offsetY;
	qreal x2 = toQReal(_attributes.value("x2", "0"));
	qreal y2 = toQReal(_attributes.value("y2", "0")) + _offsetY;
	if (x1 != x2 || x2 != y2) {
		painter->setPen(QPen(QBrush(QColor(strokeColor)), width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		painter->setBrush(Qt::NoBrush);
		painter->drawLine( QPointF(x1, y1), QPointF(x2, y2) );
	}
	return y1 > y2 ? y1 : y2;
}

// Circle
PdfElementCircle::PdfElementCircle() : PdfElement() {}
PdfElementCircle::~PdfElementCircle() {}
qreal PdfElementCircle::paint(QPainter *painter) {
	qreal width = _attributes.value("stroke", "2").toFloat();
	QString strokeColor = _attributes.value("strokecolor", "black");
	QString fillColor = _attributes.value("fillcolor", "");
	qreal cx = toQReal(_attributes.value("cx", "0"));
	qreal cy = toQReal(_attributes.value("cy", "0")) + _offsetY;
	qreal rx = toQReal(_attributes.value("r", "0"));
	qreal ry = toQReal(_attributes.value("r", "0"));
	if (rx > 0 && ry > 0) {
		painter->setPen(QPen(QBrush(QColor(strokeColor)), width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		if (!fillColor.isEmpty()) {
			painter->setBrush(QBrush(QColor(fillColor)));
		} else {
			painter->setBrush(Qt::NoBrush);
		}
		painter->drawEllipse( QPointF(cx, cy), rx, ry );
	}
	return cy + ry;
}

// Ellipse
PdfElementEllipse::PdfElementEllipse() : PdfElement() {}
PdfElementEllipse::~PdfElementEllipse() {}
qreal PdfElementEllipse::paint(QPainter *painter) {
	qreal width = _attributes.value("stroke", "2").toFloat();
	QString strokeColor = _attributes.value("strokecolor", "black");
	QString fillColor = _attributes.value("fillcolor", "");
	qreal cx = toQReal(_attributes.value("cx", "0"));
	qreal cy = toQReal(_attributes.value("cy", "0")) + _offsetY;
	qreal rx = toQReal(_attributes.value("rx", "0"));
	qreal ry = toQReal(_attributes.value("ry", "0"));
	if (rx > 0 && ry > 0) {
		painter->setPen(QPen(QBrush(QColor(strokeColor)), width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		if (!fillColor.isEmpty()) {
			painter->setBrush(QBrush(QColor(fillColor)));
		} else {
			painter->setBrush(Qt::NoBrush);
		}
		painter->drawEllipse( QPointF(cx, cy), rx, ry );
	}
	return cy + ry;
}

// Arc
PdfElementArc::PdfElementArc() : PdfElement() {}
PdfElementArc::~PdfElementArc() {}
qreal PdfElementArc::paint(QPainter *painter) {
	// TODO: Implement, see also Chord and Pie
	return _offsetY;
}

// Rectangle
PdfElementRectangle::PdfElementRectangle() : PdfElement() {}
PdfElementRectangle::~PdfElementRectangle() {}
qreal PdfElementRectangle::paint(QPainter *painter) {
	qreal width = _attributes.value("stroke", "2").toFloat();
	QString strokeColor = _attributes.value("strokecolor", "black");
	QString fillColor = _attributes.value("fillcolor", "");
	qreal x = toQReal(_attributes.value("x", "0"));
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal w = toQReal(_attributes.value("width", "0"));
	qreal h = toQReal(_attributes.value("height", "0"));
	if (w > 0 && h > 0) {
		painter->setPen(QPen(QBrush(QColor(strokeColor)), width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		if (!fillColor.isEmpty()) {
			painter->setBrush(QBrush(QColor(fillColor)));
		} else {
			painter->setBrush(Qt::NoBrush);
		}
		painter->drawRect( QRectF(QPointF(x, y), QSizeF(w, h)) );
	}
	return y + h;
}

// Polygon
PdfElementPolygon::PdfElementPolygon() : PdfElement() {}
PdfElementPolygon::~PdfElementPolygon() {}
qreal PdfElementPolygon::paint(QPainter *painter) {
	qreal back = -1;
	qreal width = _attributes.value("stroke", "2").toFloat();
	QString strokeColor = _attributes.value("strokecolor", "black");
	QString fillColor = _attributes.value("fillcolor", "");
	bool close = _attributes.value("close", "true").toLower() == "true";
	QStringList xl = _attributes.value("x", "").split(",", QString::SkipEmptyParts);
	QStringList yl = _attributes.value("y", "").split(",", QString::SkipEmptyParts);
	if ((xl.size() > 0) && (xl.size() == yl.size())) {
		painter->setPen(QPen(QBrush(QColor(strokeColor)), width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
		if (!fillColor.isEmpty()) {
			painter->setBrush(QBrush(QColor(fillColor)));
		} else {
			painter->setBrush(Qt::NoBrush);
		}
		QPolygonF polygon;
		for (int i = 0; i < xl.size(); i++) {
			polygon << QPointF(toQReal(xl.at(i)), toQReal(yl.at(i)) + _offsetY);
			back = polygon.last().y() > back ? polygon.last().y() : back;
		}
		if (close) {
			painter->drawPolygon(polygon);
		} else {
			painter->drawPolyline(polygon);
		}
	}
	return back;
}

// Text
PdfElementText::PdfElementText() : PdfElement() {}
PdfElementText::~PdfElementText() {}
qreal PdfElementText::paint(QPainter *painter) {
	qreal back = -1;
	qreal width = toQReal(_attributes.value("stroke", "2"));
	QString strokeColor = _attributes.value("strokecolor", "black");
	QString fillColor = _attributes.value("fillcolor", "");
	QString valign = _attributes.value("valign", "justify");
	QString halign = _attributes.value("halign", "top");
	QString fontfamily = _attributes.value("font", "Aller light");
	QString weight = _attributes.value("weight", "normal");
	qreal size = _attributes.value("size", "12").toFloat();
	bool italic = _attributes.value("italic", "false").toLower() == "true";
	bool underline = _attributes.value("underline", "false").toLower() == "true";
	bool wordwrap = _attributes.value("wordwrap", "true").toLower() == "true";
	qreal x = toQReal(_attributes.value("x", "0"));
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal w = toQReal(_attributes.value("width", "0"));
	qreal h = toQReal(_attributes.value("height", "0"));
	qreal s = toQReal(_attributes.value("spacing", "0"));
	
	if (w > 0 && h > 0) {
		QFont font = QApplication::font();
		font.setFamily(fontfamily);
		if (weight == "light") {
			font.setWeight(QFont::Light);
		} else if (weight == "normal") {
			font.setWeight(QFont::Normal);
		} else if (weight == "demibold") {
			font.setWeight(QFont::DemiBold);
		} else if (weight == "bold") {
			font.setWeight(QFont::Bold);
		} else if (weight == "black") {
			font.setWeight(QFont::Black);
		} else {
			font.setWeight(QFont::Normal);
		}
		font.setItalic(italic);
		font.setUnderline(underline);
		font.setPointSizeF(size);
		
		QPen pen(Qt::black, width, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
		painter->setPen(pen);
		painter->setBrush(Qt::black);
		painter->setFont(font);
		
		int flags = Qt::TextDontClip;
		if (halign == "left") flags |= Qt::AlignLeft;
		else if (halign == "right") flags |= Qt::AlignRight;
		else if (halign == "justify") flags |= Qt::AlignJustify;
		else if (halign == "center") flags |= Qt::AlignHCenter;
		
		if (valign == "top") flags |= Qt::AlignTop;
		else if (valign == "bottom") flags |= Qt::AlignBottom;
		else if (valign == "center") flags |= Qt::AlignVCenter;
		
		if (wordwrap) flags |= Qt::TextWordWrap;
		
		// Replace repeating variables and static ones after
		QString cdata = _attributes.value("cdata", "");
		QHash<QString, QString>::const_iterator it;
		if (_repeating != 0) {
			for (it = _repeating->constBegin(); it != _repeating->constEnd(); it++) {
				cdata.replace(QString("{%1}").arg(it.key()), it.value(), Qt::CaseInsensitive);
			}
		}
		if (_variables != 0) {
			for (it = _variables->constBegin(); it != _variables->constEnd(); it++) {
				cdata.replace(QString("{%1}").arg(it.key()), it.value(), Qt::CaseInsensitive);
			}
		}
		
		// split into different paragraphs
		QStringList paragraphs = cdata.split(QRegExp("(\r|\n)"), QString::SkipEmptyParts);
		
		// paint the text
		QRectF *bounding = new QRectF;
		QRectF rect = QRectF(QPointF(x, y), QSizeF(w, h));
		for (int i = 0; i < paragraphs.size(); i++) {
			painter->drawText(rect, flags, paragraphs.at(i).trimmed(), bounding);
			rect.adjust(0, bounding->height() + s, 0, 0);
		}
		back = y + (paragraphs.size() * ((size * 12.5) + s));
	}
	return back;
}

// Image
PdfElementImage::PdfElementImage() : PdfElement() {}
PdfElementImage::~PdfElementImage() {}
qreal PdfElementImage::paint(QPainter *painter) {
	QString image = _templatePath.append("/").append(_attributes.value("file", ""));
	qreal x = toQReal(_attributes.value("x", "0"));
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal w = toQReal(_attributes.value("width", "0"));
	qreal h = toQReal(_attributes.value("height", "0"));
	if (w > 0 && h > 0) {
		QImage picture;
		if (picture.load(image)) {
			painter->setPen(Qt::NoPen);
			painter->setBrush(Qt::NoBrush);
			painter->drawImage( QRectF(QPointF(x, y), QSizeF(w, h)), picture, QRectF(picture.rect()) );
		}
	}
	return y + h;
}