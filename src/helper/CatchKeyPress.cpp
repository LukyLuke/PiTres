/*
    <one line to give the program's name and a brief idea of what it does.>
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


#include "CatchKeyPress.h"

#include <QDebug>

CatchKeyPress::CatchKeyPress(QObject *parent) : QObject(parent) {
	
}

bool CatchKeyPress::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() ==  QEvent::KeyPress) {
		QKeyEvent *k = static_cast<QKeyEvent *>(event);
		Qt::Key _k = (Qt::Key) k->key();
		if (_k == Qt::Key_Return) {
			emit returnPressed();
		}
		emit keyPressed(_k);
	}
	return QObject::eventFilter(obj, event);
}

