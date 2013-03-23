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


#include "FIOCalc.h"

FIOCalc::FIOCalc() { }

FIOCalc::~FIOCalc() { }

void FIOCalc::registerSections(QHash <QString, float>sections) {
	QHash<QString, float>::const_iterator it = sections.constBegin();
	while (it !=  sections.constEnd()) {
		addSection(it.key(), it.value());
		it++;
	}
}

void FIOCalc::addSection(QString name, float amount) {
	sections.insert(name, amount);
}

void FIOCalc::reset() {
	sections.clear();
	f_total = 0;
}

QHash<QString, float> FIOCalc::calculate(float amount) {
	QHash<QString, float> payments;
	QHash<QString, float> copy(sections);

	// Calculate the total recommendation
	f_total = 0;
	foreach (float val, copy) {
		f_total += val;
	}

	// calculate all parts
	_calc(&copy, &payments, amount);
	return payments;
}

void FIOCalc::_calc(QHash<QString, float> *copy, QHash<QString, float> *payments, float amount) {
	// If there is nothing to share, just return
	if (amount <= 0) {
		return;
	}

	// If there is at least one section to get a piece, calculate it
	if (copy->size() > 0) {
		QHash<QString, float>::const_iterator it = copy->constBegin();

		// If the amount is bigger than the total recommended amount, give each section the procentual amount
		if (amount > f_total) {
			while (it != copy->constEnd()) {
				// The amount for the section is called by: amount * %-section-of-total
				payments->insert( it.key(), (amount * (it.value() / f_total)) );
				it++;
			}
			copy->clear();
			return;
		}

		// Get the smallest section amount, share it and calculate the dept due
		float due = 0, smallest = 0;
		while (it != copy->constEnd()) {
			smallest = (smallest <= 0 || it.value() < smallest) ? it.value() : smallest;
			it++;
		}

		// If the partial amount of the outstanding amount divided by the number of sections,
		// is smaller than the smallest amount, we ned no more calculation
		due = amount / copy->size();
		if (due <= smallest) {
			it = copy->begin();
			while (it != copy->constEnd()) {
				payments->insert( it.key(), due );
				it++;
			}
			copy->clear();
			return;
		}

		// Pay out the smallest amount to the given section and rerun the calculation
		it = copy->begin();
		while (it != copy->end()) {
			if (smallest >= it.value()) {
				payments->insert( it.key(), it.value() );
				amount -= it.value();
				copy->remove( it.key() );
				it = copy->begin();
			} else {
				it++;
			}
		}
		_calc(copy, payments, amount);
	}
}
