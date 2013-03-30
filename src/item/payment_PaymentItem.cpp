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
		s_reference = "";
		d_payable.setDate(-1, -1, -1);
		d_valuta.setDate(-1, -1, -1);
		d_paiddue.setDate(-1, -1, -1);
		s_name = "";
		i_member = 0;
		s_section = "";
		f_amountPaidTotal = 0;
		f_amountPaid = 0;
		f_amount = 0;
	}

	PaymentItem::PaymentItem(const PaymentItem &other) : QObject(other.parent()) {
		formattedString = other.formattedString;
		s_reference = other.s_reference;
		d_payable = other.d_payable;
		d_valuta = other.d_valuta;
		d_paiddue = other.d_paiddue;
		s_name = other.s_name;
		i_member = other.i_member;
		s_section = other.s_section;
		f_amountPaid = other.f_amountPaid;
		f_amountPaidTotal = other.f_amountPaidTotal;
		f_amount = other.f_amount;
	}


	PaymentItem::~PaymentItem() {
	}

	QString PaymentItem::formatted() {
		return formattedString.arg(
			reference(),
			payable().toString("yyyy-MM-dd"),
			name(),
			QString::number(member()),
			section(),
			QString::number(amountPaidTotal()),
			QString::number(amount())
		);
	}

	QString PaymentItem::getData(qint32 column) {
		switch (column) {
			case 0:  return reference();
			case 1:  return QString::number(member());
			case 2:  return name();
			case 3:  return section();
			case 4:  return tr("%1 sFr.").arg(amountPaid(), 0, 'f', 2);
			case 5:  return valuta().toString("yyyy-MM-dd");
			case 6:  return tr("%1 sFr.").arg(amount(), 0, 'f', 2);
			case 7:  return payable().toString("yyyy-MM-dd");
			case 8:  return paidDue().toString("yyyy-MM-dd");
			case 9:  return tr("%1 sFr.").arg(amountPaidTotal(), 0, 'f', 2);
			default: return formatted();
		}
	}
	QString PaymentItem::headerData(qint32 column) {
		switch (column) {
			case 0:  return tr("Reference");
			case 1:  return tr("Member");
			case 2:  return tr("Name");
			case 3:  return tr("Section");
			case 4:  return tr("Paid");
			case 5:  return tr("Valuta");
			case 6:  return tr("Proposed");
			case 7:  return tr("Payable due");
			case 8:  return tr("Payed due");
			case 9:  return tr("Total paid");
			default: return tr("Formatted value");
		}
	}

	void PaymentItem::setReference(QString reference) {
		s_reference = reference;
	}

	void PaymentItem::setPayable(QDate payable) {
		d_payable = payable;
	}

	void PaymentItem::setValuta(QDate valuta) {
		d_valuta = valuta;
	}

	void PaymentItem::setPaidDue(QDate paiddue) {
		d_paiddue = paiddue;
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

	void PaymentItem::setAmountPaidTotal(float amount) {
		f_amountPaidTotal = amount;
	}
	
	void PaymentItem::setAmount(float amount) {
		f_amount = amount;
	}
}
