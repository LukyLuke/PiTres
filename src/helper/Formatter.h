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

#ifndef Helper_Formatter_H 
#define Helper_Formatter_H 

#include <cmath>
#include <QString>
#include <QLocale>

class Formatter {
public:
	static QString telephone(QString number);
	static QString telephone(QString number, QLocale::Country country);
	static QString email(QString email);
	static QString number(float number, int precision = 2);
	static QString currency(float amount);
	static QLocale::Country iso3166country(QString iso639);
};

#endif // Helper_Formatter_H 
