/*
	A reminder - based on a Invoice (not sure if needed)
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

#ifndef Data_Reminder_H
#define Data_Reminder_H

#include "Invoice.h"
#include "../helper/XmlPdf.h"

class Reminder : public Invoice {
public:
	Reminder(QObject *parent = 0);
	virtual ~Reminder();
};

#endif // Data_Reminder_H 
