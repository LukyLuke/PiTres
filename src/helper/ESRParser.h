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

#ifndef ESR_Parser_h
#define ESR_Parser_h

#include <QString>
#include <QDate>
#include <QRegExp>
#include <QList>
#include <QStringList>

enum esr_type3 {
	ESR_RECEIVE_NO_VOUCHER = 2,
	ESR_CANCELLATION_NO_VOUCHER = 5,
	ESR_CORRECTION_NO_VOUCHER = 8,
	
	ESR_RECEIVE_COUNTER = 12,
	ESR_CANCELLATION_COUNTER = 15,
	ESR_CORRECTION_COUNTER = 18,
	
	ESR_RECEIVE_CASH_ON_DELIVERY = 22,
	ESR_CANCELLATION_CASH_ON_DELIVERY = 25,
	ESR_CORRECTION_CASH_ON_DELIVERY = 28,
	
	ESR_RECEIVE_OWN_ACCOUNT = 32,
	ESR_CANCELLATION_OWN_ACCOUNT = 35,
	ESR_CORRECTION_OWN_ACCOUNT = 38,
	
	ESRPLUS_RECEIVE_NO_VOUCHER = 102,
	ESRPLUS_CANCELLATION_NO_VOUCHER = 105,
	ESRPLUS_CORRECTION_NO_VOUCHER = 108,

	ESRPLUS_RECEIVE_COUNTER = 112,
	ESRPLUS_CANCELLATION_COUNTER = 115,
	ESRPLUS_CORRECTION_COUNTER = 118,

	ESRPLUS_RECEIVE_OWN_ACCOUNT = 132,
	ESRPLUS_CANCELLATION_OWN_ACCOUNT = 135,
	ESRPLUS_CORRECTION_OWN_ACCOUNT = 138,

	ESR_TOTAL_RECEIVE_OR_CORRECTION = 999,
	ESR_TOTAL_STORNO = 995
};

enum esr_type4 {
	ESR_CHF_DEFAULT = 1,
	ESR_CHF_CASH_ON_DELIVERY = 2,
	ESR_CHF_OWN_ACCOUNT = 3,

	ESRPLUS_CHF_DEFAULT = 11,
	ESRPLUS_CHF_OWN_ACCOUNT = 13,

	ESR_EUR_DEFAULT = 21,
	ESR_EUR_OWN_ACCOUNT = 23,

	ESRPLUS_EUR_DEFAULT = 31,
	ESRPLUS_EUR_OWN_ACCOUNT = 33
};

enum esr_trans_type4 {
	ESR_TRANSACTION_RECEIVE = 1,
	ESR_TRANSACTION_STORNO = 2,
	ESR_TRANSACTION_CORRECTION = 3
};

enum esr_origin {
	ESR_ORIGIN_POSTOFFICE = 1,
	ESR_ORIGIN_ZAG_DAG = 2,
	ESR_ORIGIN_NO_VOUCHER = 3,
	ESR_ORIGIN_EURO_SIC = 4
};
enum esr_delivery {
	ESR_DELIVERY_ORIGINAL = 1,
	ESR_DELIVERY_REKO = 2,
	ESR_DELIVERY_TEST = 3
};

enum esr_reject {
	ESR_NO_REJECT = 0,
	ESR_REJECT = 1,
	ESR_MASS_REJECT = 5
};

struct esr_data_type3 {
	esr_type3 type; // start: 0, count: 3
	QString esr_account; // start: 3, count: 9
	QString reference_number; // start: 12, count: 27
	float amount; // start: 39, count: 10 (8 if esr_account is only 5 chars; amount then followed by '99')
	QString office_reference; // start: 49, count: 10
	QDate deposit_date; // start: 59, count: 6, format: yyMMdd
	QDate processing_date; // start: 65, count: 6, format: yyMMdd
	QDate valuta_date; // start: 71, count: 6, format: yyMMdd
	qint32 microfilm_number; // start: 77, count: 9
	esr_reject reject; // start: 86, count: 1
	QString blank; // start: 87, count: 9
	float coast; // start: 96, count: 2, followed by '99'
};

struct esr_data_type4 {
	esr_type4 transaction_code; // start: 0, count: 2
	esr_trans_type4 transaction; // start: 2, count: 1
	esr_origin origin; // start: 3, count: 2
	esr_delivery delivery_type; // start: 5, count: 1
	QString esr_account; // start: 6, count: 9
	QString reference_number; // start: 15, count: 27
	QString currency; // start: 42, count: 3
	float amount; // start: 45, count: 10, followed by '99'
	QString office_reference; // start: 57, count: 35
	QDate deposit_date; // start: 92, count: 8, format: yyyyMMdd
	QDate processing_date; // start: 100, count: 8, format: yyyyMMdd
	QDate valuta_date; // start: 108, count: 8, format: yyyyMMdd
	esr_reject reject; // start: 116, count: 1
	QString coast_currency; // start: 117, count: 3
	float coast; // start: 120, count: 4, followed by '99'
	QString blank; // start: 126, count: 74
};

struct esr_total_type3 {
	esr_type3 type; // start: 0, count: 3
	QString esr_account; // start: 3, count: 9
	float amount; // start: 39, count: 10, followed by '99'
	qint32 num_transactions; // start: 51, count: 12
	QDate date; // start: 63, count: 6, format: yyMMdd
	float coast; // start: 69, count: 7, followed by '99'
	float esrplus_coast; // start: 78, count: 7, followed by '99'
	QString blank; // start: 87, count: 13
};

struct esr_total_type4 {
	esr_type4 transaction_code; // start: 0, count: 2
	esr_trans_type4 transaction; // start: 2, count: 1
	esr_origin origin; // start: 3, count: 2
	esr_delivery delivery_type; // start: 5, count: 1
	QString esr_account; // start: 6, count: 9
	QString reference_number; // start: 15, count: 27
	QString currency; // start: 42, count: 3
	float amount; // start: 45, count: 10, followed by '99'
	qint32 num_transactions; // start: 57, count: 12
	QDate date; // start: 69, count: 8, format: yyyyMMdd
	QString coast_currency; // start: 77, count: 3
	float coast; // start: 80, count: 9, followed by '99'
	QString blank; // start: 91, count: 109 (blank or spaces)
};

struct esr_record_3 {
	QList<esr_data_type3> data;
	esr_total_type3 total;
};

struct esr_record_4 {
	QList<esr_data_type4> data;
	esr_total_type4 total;
};

/**
 * Parse the given data line by line and return a esr_record_3 structure
 * 
 * @param QString data Data to parse line by line
 * @return esr_record_3
 */
esr_record_3 parse_esr3(const QString data);

/**
 * Parse the given data line by line and return a esr_record_4 structure
 *
 * @param QString data Data to parse line by line
 * @return esr_record_4
 */
esr_record_4 parse_esr4(const QString data);

#endif // ESR_Parser_h
