/*
	Special delegate which can be used to show a date in an appropriate format.
	Copyright (C) 2013  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

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

#include "DateDelegate.h"

DateDelegate::DateDelegate(QString format, QWidget *parent) :
	QStyledItemDelegate(parent),
	s_format(format) {
}

DateDelegate::~DateDelegate() {}

QString DateDelegate::displayText(const QVariant &value, const QLocale & /*locale*/) const {
	return value.toDate().toString(s_format);
}

