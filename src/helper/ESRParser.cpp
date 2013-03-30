/*
	Helper functions and datatypes to parse an ESR-Record into a useable format.
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

#include "ESRParser.h"

esr_record_3 parse_esr3(const QString data) {
	esr_record_3 back;
	QString line, part;
	QStringList list = data.split(QRegExp("(\\r\\n|\\r|\\n)"), QString::SkipEmptyParts);

	for (qint32 i = 0; i < list.size(); i++) {
		line = list.at(i);
		if (i <= list.size()) {
			esr_data_type3 data;
			data.type = (esr_type3)line.mid(0, 3).toInt();
			data.esr_account = line.mid(3, 9);
			data.reference_number = line.mid(12, 27);
			data.amount = line.mid(39, 10).toFloat() * 0.01;
			data.office_reference = line.mid(49, 10);
			data.deposit_date = QDate::fromString(line.mid(59, 6), "yyMMdd");
			data.processing_date = QDate::fromString(line.mid(65, 6), "yyMMdd");
			data.valuta_date = QDate::fromString(line.mid(71, 6), "yyMMdd");
			data.microfilm_number = line.mid(77, 9).toInt();
			data.reject = (esr_reject)line.mid(86, 1).toInt();
			data.blank = line.mid(87, 9);
			data.coast = line.mid(96, 2).toFloat() * 0.01;
			back.data.append(data);

		} else {
			back.total.type = (esr_type3)line.mid(0, 3).toInt();
			back.total.esr_account = line.mid(3, 9);
			back.total.amount = line.mid(39, 10).toFloat() * 0.01;
			back.total.num_transactions = line.mid(51, 12).toInt();
			back.total.date = QDate::fromString(line.mid(63, 6), "yyMMdd");
			back.total.coast = line.mid(96, 7).toFloat() * 0.01;
			back.total.esrplus_coast = line.mid(78, 7).toFloat() * 0.01;
			back.total.blank = line.mid(87, 13);
		}
	}
	return back;
}

esr_record_4 parse_esr4(const QString data) {
	esr_record_4 back;
	QString line, part;
	QStringList list = data.split(QRegExp("(\\r\\n|\\r|\\n)"), QString::SkipEmptyParts);

	for (qint32 i = 0; i < list.size(); i++) {
		line = list.at(i);
		if (i <= list.size()) {
			esr_data_type4 data;
			data.transaction_code = (esr_type4)line.mid(0, 2).toInt();
			data.transaction = (esr_trans_type4)line.mid(2, 1).toInt();
			data.origin = (esr_origin)line.mid(3, 2).toInt();
			data.delivery_type = (esr_delivery)line.mid(5, 1).toInt();
			data.esr_account = line.mid(6, 9);
			data.reference_number = line.mid(15, 27);
			data.currency = line.mid(42, 3).toUpper();
			data.amount = line.mid(45, 10).toFloat() * 0.01;
			data.office_reference = line.mid(57, 35);
			data.deposit_date = QDate::fromString(line.mid(92, 8), "yyyyMMdd");
			data.processing_date = QDate::fromString(line.mid(100, 8), "yyyyMMdd");
			data.valuta_date = QDate::fromString(line.mid(108, 8), "yyyyMMdd");
			data.reject = (esr_reject)line.mid(116, 1).toInt();
			data.coast_currency = line.mid(117, 3).toUpper();
			data.coast = line.mid(120, 4).toFloat() * 0.01;
			data.blank = line.mid(126, 74);
			back.data.append(data);

		} else {
			back.total.transaction_code = (esr_type4)line.mid(0, 2).toInt();
			back.total.transaction = (esr_trans_type4)line.mid(2, 1).toInt();
			back.total.origin = (esr_origin)line.mid(3, 2).toInt();
			back.total.delivery_type = (esr_delivery)line.mid(5, 1).toInt();
			back.total.esr_account = line.mid(6, 9);
			back.total.reference_number = line.mid(15, 27);
			back.total.currency = line.mid(42, 3).toUpper();
			back.total.amount = line.mid(45, 10).toFloat() * 0.01;
			back.total.num_transactions = line.mid(57, 12).toInt();
			back.total.date = QDate::fromString(line.mid(69, 8), "yyyyMMdd");
			back.total.coast_currency = line.mid(77, 3).toUpper();
			back.total.coast = line.mid(80, 9).toFloat() * 0.01;
			back.total.blank = line.mid(91, 109);
		}
	}
	return back;
}
