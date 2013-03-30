/*
	ModelItem for PaymentImplort.
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


#include "payment_PaymentItem.h"

namespace payment {

	PaymentItem::PaymentItem(QObject *parent) : QObject(parent) {
		formattedString = tr("%1 (%2): %3 (%4) / Section %5 / %6 of %7 paid");
	}

	PaymentItem::PaymentItem(const PaymentItem &other) : QObject(other.parent()) {
		formattedString = other.formattedString;
		s_reference = other.s_reference;
		d_payable = other.d_payable;
		s_name = other.s_name;
		i_member = other.i_member;
		s_section = other.s_section;
		f_amountPaid = other.f_amountPaid;
		f_amount = other.f_amount;
	}


	PaymentItem::~PaymentItem() {
	}

	QString PaymentItem::formatted() {
		return formattedString.arg(
			s_reference,
			d_payable.toString("yyyy-MM-dd"),
			s_name,
			QString::number(i_member),
			s_section,
			QString::number(f_amountPaid),
			QString::number(f_amount)
		);
	}

	void PaymentItem::setReference(QString reference) {
		s_reference = reference;
	}
	
	void PaymentItem::setPayable(QDate payable) {
		d_payable = payable;
	}
	
	void PaymentItem::setName(QString name) {
		s_name = name;
	}
	
	void PaymentItem::setMember(int member) {
		i_member = member;
	}
	
	void PaymentItem::setSection(QString section) {
		s_section = section;
	}
	
	void PaymentItem::setAmountPaid(float amount) {
		f_amountPaid = amount;
	}
	
	void PaymentItem::setAmount(float amount) {
		f_amount = amount;
	}
}
