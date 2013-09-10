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
#include <QtSvg/QtSvg>
#include <QtSvg/QSvgRenderer>
#include <QDebug>

PdfElement::PdfElement() {
	_offset = 0;
	_top = 0;
	_onlyOnFirst= false;
	_onlyOnLast = false;
	_onlyOnNth = 0;
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
	_onlyOnFirst= newPdfElement._onlyOnFirst;
	_onlyOnLast = newPdfElement._onlyOnLast;
	_onlyOnNth = newPdfElement._onlyOnNth;
	_expressions = newPdfElement._expressions;
}

PdfElement::~PdfElement() {}

qreal PdfElement::paint(QPainter *painter, const qint32 row, const qint32 maxrows) {
	PdfElementLine *_line;
	PdfElementCircle *_circle;
	PdfElementArc *_arc;
	PdfElementEllipse *_ellipse;
	PdfElementRectangle *_rectangle;
	PdfElementPolygon *_polygon;
	PdfElementText *_text;
	PdfElementImage *_image;
	qreal back = 0, bottom = 0;
	
	// Return if this Element-Group should not be printed
	if ((row > 0) && !checkShow(row, maxrows)) {
		return 0;
	}
	
	// Print all Elements in this group
	for (int i = 0; i < _nodes.size(); i++) {
		PdfElement e = _nodes.at(i);
		if ((row > 0) && !e.checkShow(row, maxrows)) {
			continue;
		}
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
			case PdfUnknown:
			default:
				back = 0;
				break;
		}
		bottom = back > bottom ? back : bottom;
	}
	return bottom + _offset;
}

qreal PdfElement::bottom(const qint32 row, const qint32 maxrows) {
	PdfElementLine *_line;
	PdfElementCircle *_circle;
	PdfElementArc *_arc;
	PdfElementEllipse *_ellipse;
	PdfElementRectangle *_rectangle;
	PdfElementPolygon *_polygon;
	PdfElementText *_text;
	PdfElementImage *_image;
	qreal back = 0, _bottom = 0;
	
	// Return if this Element-Group should not be printed
	if ((row > 0) && !checkShow(row, maxrows)) {
		return -1;
	}
	
	// Print all Elements in this group
	for (int i = 0; i < _nodes.size(); i++) {
		PdfElement e = _nodes.at(i);
		if ((row > 0) && !e.checkShow(row, maxrows)) {
			continue;
		}
		e.setVars(_variables, _repeating);
		e.setTop(_offsetY);
		switch (e.type()) {
			case PdfLine:
				_line = (PdfElementLine *)&e;
				back = _line->bottom();
				break;
			case PdfCircle:
				_circle = (PdfElementCircle *)&e;
				back = _circle->bottom();
				break;
			case PdfArc:
				_arc = (PdfElementArc *)&e;
				back = _arc->bottom();
				break;
			case PdfEllipse:
				_ellipse = (PdfElementEllipse *)&e;
				back = _ellipse->bottom();
				break;
			case PdfRectangle:
				_rectangle = (PdfElementRectangle *)&e;
				back = _rectangle->bottom();
				break;
			case PdfPolygon:
				_polygon = (PdfElementPolygon *)&e;
				back = _polygon->bottom();
				break;
			case PdfText:
				_text = (PdfElementText *)&e;
				back = _text->bottom();
				break;
			case PdfImage:
				_image = (PdfElementImage *)&e;
				back = _image->bottom();
				break;
			case PdfUnknown:
			default:
				back = 0;
				break;
		}
		_bottom = back > _bottom ? back : _bottom;
	}
	return _bottom + _offset;
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
	
	_onlyOnLast = false;
	_onlyOnFirst = false;
	_onlyOnNth = 0;
	if (element.hasAttribute("nth") && !element.attribute("nth").isEmpty()) {
		_onlyOnLast = element.attribute("nth").toLower() == "last";
		_onlyOnFirst = element.attribute("nth").toLower() == "first";
		_onlyOnNth = _onlyOnLast || _onlyOnFirst ? 1 : element.attribute("nth").toInt();
	}
	
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

bool PdfElement::checkShow(const qint32 row, const qint32 maxrows) const {
	return (
		(_onlyOnLast && (row == maxrows)) ||
		(_onlyOnFirst && (row == 1)) ||
		(!_onlyOnLast && !_onlyOnFirst && (_onlyOnNth > 0) && (row % _onlyOnNth == 0)) ||
		(_onlyOnNth <= 0)
	);
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
	if (cdata != NULL && !cdata.isEmpty()) {
		extractExpressions(const_cast<QString*>(&cdata));
		_attributes.insert("cdata", cdata);
	}
	QString val;
	for (uint i = 0; i < attr.length(); i++) {
		QDomAttr a = attr.item(i).toAttr();
		if (!a.isNull()) {
			if (a.name().toLower() == "nth") {
				_onlyOnLast = a.value().toLower() == "last";
				_onlyOnFirst = a.value().toLower() == "first";
				_onlyOnNth = _onlyOnLast || _onlyOnFirst ? 1 : a.value().toInt();
			}
			val = a.value();
			extractExpressions(const_cast<QString*>(&val));
			_attributes.insert(a.name().toLower(), val);
		}
	}
}

void PdfElement::extractExpressions(QString *cdata) {
	// Search for "{IF variable exp 'value'}...{ELSE}...{/IF}"
	QRegExp rx("\\{IF\\s+([A-Z0-9_-]+)\\s+(\\<|\\>|\\<=|\\>=|==|\\<\\>|\\!=|\\~=)\\s+(\"|\\')([^\\}]*)(\\3)\\}(.*)\\{ELSE\\}(.*)\\{\\/IF\\}");
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	// Offsets:
	// 0: Replacement
	// 1: Variable name
	// 2: expression
	// 3: " or ' to enclose the value to check against
	// 4: Value to check against
	// 5: " or ' to enclose the value to check against
	// 6: Value if expresion succeed
	// 7: Value if expresion failes

	// Search for all occurences
	int offset = 0;
	while ((offset = rx.indexIn(*cdata, offset)) != -1) {
		QStringList match = rx.capturedTexts();
		QString expr;
		if (match.size() >= 6) {
			expression e;
			e.value = match.at(4);
			e.successfull = match.value(6).trimmed();
			e.otherwise = match.value(7).trimmed();
			e.variable = match.at(1);
			e.expr = NULL;
			expr = match.at(2);
			e.expr = (
				(expr == "<" ? &expr_lt :
				(expr == ">" ? &expr_gt :
				(expr == "<=" ? &expr_le :
				(expr == ">=" ? &expr_ge :
				(expr == "==" ? &expr_eq :
				(expr == "<>" || expr == "!=" ? &expr_ne :
				(expr == "~=" ? &expr_contains : NULL)
			)))))));

			// Only if we have a valid expression we use it
			if (e.expr != NULL) {
				QString key("{IF:");
				key.append(QString::number(_expressions.size())).append("}");
				_expressions.insert(key, e);
				cdata->replace(match.at(0), key);
				offset += key.length();
				continue;
			}
		}
		// Not count up here if we  found a valid match - see continue above
		offset += rx.matchedLength();
	}
}

void PdfElement::evalExpression(QString *cdata, const QString &var, const QString &val) {
	if (cdata->contains("{IF:")) {
		int offset = 0;
		QRegExp rx("\\{IF:\\d+\\}");
		rx.setCaseSensitivity(Qt::CaseSensitive);
		QString match, res;
		while ((offset = rx.indexIn(*cdata, offset)) != -1) {
			match = rx.capturedTexts().at(0);
			expression e = _expressions.value(match);
			if (e.valid() && (var.compare(e.variable, Qt::CaseInsensitive) == 0)) {
				res = (e.expr(&val, &e.value) ? e.successfull : e.otherwise);
				cdata->replace(match, res);
				offset += res.size();
			} else {
				offset += rx.matchedLength();
			}
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
	qreal width = toQReal(_attributes.value("stroke", "2"));
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
	return bottom();
}
qreal PdfElementLine::bottom() {
	qreal y1 = toQReal(_attributes.value("y1", "0")) + _offsetY;
	qreal y2 = toQReal(_attributes.value("y2", "0")) + _offsetY;
	return y1 > y2 ? y1 : y2;
}

// Circle
PdfElementCircle::PdfElementCircle() : PdfElement() {}
PdfElementCircle::~PdfElementCircle() {}
qreal PdfElementCircle::paint(QPainter *painter) {
	qreal width = toQReal(_attributes.value("stroke", "2"));
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
	return bottom();
}
qreal PdfElementCircle::bottom() {
	qreal cy = toQReal(_attributes.value("cy", "0")) + _offsetY;
	qreal ry = toQReal(_attributes.value("r", "0"));
	return cy + ry;
}

// Ellipse
PdfElementEllipse::PdfElementEllipse() : PdfElement() {}
PdfElementEllipse::~PdfElementEllipse() {}
qreal PdfElementEllipse::paint(QPainter *painter) {
	qreal width = toQReal(_attributes.value("stroke", "2"));
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
	return bottom();
}
qreal PdfElementEllipse::bottom() {
	qreal cy = toQReal(_attributes.value("cy", "0")) + _offsetY;
	qreal ry = toQReal(_attributes.value("ry", "0"));
	return cy + ry;
}

// Arc
PdfElementArc::PdfElementArc() : PdfElement() {}
PdfElementArc::~PdfElementArc() {}
qreal PdfElementArc::paint(QPainter *painter) {
	// TODO: Implement, see also Chord and Pie
	return bottom();
}
qreal PdfElementArc::bottom() {
	return _offsetY;
}

// Rectangle
PdfElementRectangle::PdfElementRectangle() : PdfElement() {}
PdfElementRectangle::~PdfElementRectangle() {}
qreal PdfElementRectangle::paint(QPainter *painter) {
	qreal width = toQReal(_attributes.value("stroke", "2"));
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
	return bottom();
}
qreal PdfElementRectangle::bottom() {
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal h = toQReal(_attributes.value("height", "0"));
	return y + h;
}

// Polygon
PdfElementPolygon::PdfElementPolygon() : PdfElement() {}
PdfElementPolygon::~PdfElementPolygon() {}
qreal PdfElementPolygon::paint(QPainter *painter) {
	qreal width = toQReal(_attributes.value("stroke", "2"));
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
		}
		if (close) {
			painter->drawPolygon(polygon);
		} else {
			painter->drawPolyline(polygon);
		}
	}
	return bottom();
}
qreal PdfElementPolygon::bottom() {
	qreal back = -1, current = 0;
	QStringList yl = _attributes.value("y", "").split(",", QString::SkipEmptyParts);
	if (!yl.isEmpty()) {
		for (int i = 0; i < yl.size(); i++) {
			current = toQReal(yl.at(i)) + _offsetY;
			back = current > back ? current : back;
		}
	}
	return back;
}

// Text
PdfElementText::PdfElementText() : PdfElement() {}
PdfElementText::~PdfElementText() {}
qreal PdfElementText::paint(QPainter *painter) {
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
				// #14 Variables can occure in expressions and show different content in different situations
				evalExpression(&cdata, it.key(), it.value());

				// Replace variables with it's value
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
	}
	return bottom();
}
qreal PdfElementText::bottom() {
	qreal size = _attributes.value("size", "12").toFloat();
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal s = toQReal(_attributes.value("spacing", "0"));
	qint32 num = _attributes.value("cdata", "").count(QRegExp("(\r|\n)"));
	return y + (num * ((size * 12.5) + s));
}

// Image
PdfElementImage::PdfElementImage() : PdfElement() {}
PdfElementImage::~PdfElementImage() {}
qreal PdfElementImage::paint(QPainter *painter) {
	qreal x = toQReal(_attributes.value("x", "0"));
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal w = toQReal(_attributes.value("width", "0"));
	qreal h = toQReal(_attributes.value("height", "0"));
	
	if (w > 0 && h > 0) {
		painter->setPen(Qt::NoPen);
		painter->setBrush(Qt::NoBrush);
		QRectF box(QPointF(x, y), QSizeF(w, h));
		QImage picture;
		
		// Get the variable defined in the attribute "file"
		QString var = _attributes.value("file", "");
		bool drawn = FALSE;
		
		// No variable found, the attribute "file" might point to an imagefile
		if (_variables->value(var, "").isEmpty()) {
			
			// Load the image (or default image) and print it
			QList<QString> images = replaceVariables(var);
			QString img;
			for (int i = 0; i < images.size(); i++) {
				img = QString("/").prepend(_templatePath).append(images.at(i));
				if (picture.load(img)) {
					painter->drawImage( box, picture, QRectF(picture.rect()) );
					drawn = TRUE;
					break;
				}
			}
			
		} else {
			// if an attribute exists with the addition "_file" the string in the attribute should be iinterpreted as a file, otherwise as SVG-Content
			bool imageIsFile = _variables->contains(var) && !_variables->value(var.append("_file"), "").isEmpty();
			
			// Try to render a normal pixel image or as an SVG Image / Content
			QSvgRenderer svg;
			if (imageIsFile && picture.load(_variables->value(var))) {
				painter->drawImage( box, picture, QRectF(picture.rect()) );
				drawn = TRUE;
				
			} else if ( (imageIsFile && svg.load( _variables->value(var) )) ||
			            (!imageIsFile && svg.load( _variables->value(var).toUtf8() ))
			) {
				svg.render(painter, box);
				drawn = TRUE;
			}
		}
		
		// If the Image isn't drawn, show the default one
		if (!drawn) {
			showDefaultImage(painter, box);
		}
	}
	return bottom();
}
qreal PdfElementImage::bottom() {
	qreal y = toQReal(_attributes.value("y", "0")) + _offsetY;
	qreal h = toQReal(_attributes.value("height", "0"));
	return y + h;
}
QList<QString> PdfElementImage::replaceVariables(QString file) {
	//              variable       prefix            suffix            default
	QRegExp re("\\{([a-z_]+)(?:\\:([^:\\}]*))?(?:\\:([^:\\}]*))?(?:\\:([^:\\}]*))?\\}", Qt::CaseInsensitive);
	int pos = 0, max;
	QString var1, var2;
	QList<QString> list;
	list << file;
	
	while ((pos = re.indexIn(file, pos)) != -1) {
		// variable and empty value
		var1 = _variables->value( re.cap(1).toLower() );
		var2 = re.cap(4);
		
		// prefix
		if (!re.cap(2).isEmpty()) {
			if (!var1.isEmpty()) {
				var1.prepend(re.cap(2));
			}
			if (!var2.isEmpty()) {
				var2.prepend(re.cap(2));
			}
		}
		
		// suffix
		if (!re.cap(3).isEmpty()) {
			if (!var1.isEmpty()) {
				var1.append(re.cap(3));
			}
			if (!var2.isEmpty()) {
				var2.append(re.cap(3));
			}
		}
		
		// Replace in files
		max = list.size();
		for (int i = 0; i < max; i++) {
			// Append the current entry with the rplaced default value (var2) at the end
			// and replace the variable itself (var1) on the current entry
			list.append(QString(list.at(i)).replace(re.cap(0), var2));
			list[i].replace(re.cap(0), var1);
		}
		pos += re.matchedLength();
	}
	
	// sort the list by length and return it
	qSort(list.begin(), list.end(), PdfElement::lengthDescending);
	return list;
}
void PdfElementImage::showDefaultImage(QPainter *painter, QRectF box) {
	QSvgRenderer svg;
	QImage picture;
	QString img;
	QList<QString> images = replaceVariables(_attributes.value("default", ""));
	
	// Try to load the image as SVG and after as a pixel graphic
	for (int i = 0; i < images.size(); i++) {
		img = QString("/").prepend(_templatePath).append( images.at(i) );
		if (svg.load(img)) {
			painter->setPen(Qt::NoPen);
			svg.render(painter, box);
			break;
		} else if (picture.load( img )) {
			painter->drawImage( box, picture, QRectF(picture.rect()) );
			break;
		}
	}
}
