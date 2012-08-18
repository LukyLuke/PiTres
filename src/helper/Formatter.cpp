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

QString Formatter::telephone(QString number) {
	return Formatter::telephone(number, QLocale::Switzerland);
}

QString Formatter::telephone(QString number, QLocale::Country country) {
	number = number.replace(QRegExp("[^0-9]+"), "").trimmed();
	if (!number.startsWith("0")) {
		number.prepend("++");
	}
	/*switch (country) {
		default:  break;
	}*/
	return number;
}

