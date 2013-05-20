/*
	Functions to format different Strings into different standard formats
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

#include "Formatter.h"

#include <QString>
#include <QRegExp>
#include <QLocale>
#include <QDebug>

QString Formatter::telephone(QString number) {
	return Formatter::telephone(number, QLocale::Switzerland);
}

QString Formatter::telephone(QString number, QLocale::Country country) {
	QRegExp rx("^(\\+\\+?|00)?\\s*(\\d+\\((\\d+)\\))?([\\d\\s\\-\\.\\/\\']+)$");
	QString back("");
	
	if (number.isEmpty()) {
		return back;
	}
	
	if (rx.indexIn(number) >= 0) {
		// with international prefix
		if (rx.cap(1).length() > 0) {
			back.append(rx.cap(1).replace("+", "00").replace("++", "00"));
		} else {
			back.append("0041");
		}
		
		// with national prefix in brackets
		if (rx.cap(3).length() > 0) {
			if (rx.cap(3).at(0) == '0') {
				back.append(rx.cap(3).remove(0, 1));
			} else {
				back.append(rx.cap(3));
			}
		}
		
		// with international prefix
		if (rx.cap(4).length() > 0) {
			back.append(rx.cap(4).replace(QRegExp("[^\\d]"), ""));
		}
		
		return back;
	}
	qDebug() << "Invalid Telephone Number: " << number << " - Unknown Format...";
	return back;
}

QString Formatter::email(QString email) {
	//QRegExp rx("^$");
	return email;
}
