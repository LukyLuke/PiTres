/*
	Helper class for calculations all around the FIO of the Pirate Party Switzerland
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


#ifndef FIOCALC_H
#define FIOCALC_H

#include "math.h"

#include <QString>
#include <QHash>

class FIOCalc {

public:
	FIOCalc();
	virtual ~ FIOCalc();
	void registerSections(QHash<QString, float> sections);
	void addSection(QString name, float amount);
	void reset();
	QHash<QString, float> calculate(float amount);

private:
	float f_total;
 	QHash<QString, float> sections;
	void _calc(QHash<QString, float> *copy, QHash<QString, float> *payments, float amount);
};

#endif // FIOCALC_H
