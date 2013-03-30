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


#ifndef Payment_PaymentIten_H
#define Payment_PaymentIten_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QDebug>

namespace payment {
	class PaymentItem : QObject {
	Q_OBJECT
	Q_PROPERTY(QString reference READ reference WRITE setReference)
	Q_PROPERTY(QDate payable READ payable WRITE setPayable)
	Q_PROPERTY(QDate valuta READ valuta WRITE setValuta)
	Q_PROPERTY(QDate paidDue READ paidDue WRITE setPaidDue)
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(int member READ member WRITE setMember)
	Q_PROPERTY(QString section READ section WRITE setSection)
	Q_PROPERTY(float amountPaidTotal READ amountPaidTotal WRITE setAmountPaidTotal)
	Q_PROPERTY(float amountPaid READ amountPaid WRITE setAmountPaid)
	Q_PROPERTY(float amount READ amount WRITE setAmount)
	
	public:
		const static qint32 NUM_PROPERTIES = 10;
		static QString headerData(qint32 column);
		
		PaymentItem(QObject *parent = 0);
		PaymentItem(const PaymentItem &other);
		virtual ~PaymentItem();
		QString formatted();
		QString getData(qint32 column);
		
		void setReference(QString reference);
		void setPayable(QDate payable);
		void setValuta(QDate valuta);
		void setPaidDue(QDate paiddue);
		void setName(QString name);
		void setMember(int member);
		void setSection(QString section);
		void setAmountPaidTotal(float amount);
		void setAmountPaid(float amount);
		void setAmount(float amount);

		QString reference() { return s_reference; };
		QDate payable() { return d_payable; };
		QDate valuta() { return d_valuta; };
		QDate paidDue() { return d_paiddue; };
		QString name() { return s_name; };
		int member() { return i_member; };
		QString section() { return s_section; };
		float amountPaidTotal() { return f_amountPaidTotal + f_amountPaid; };
		float amountPaid() { return f_amountPaid; };
		float amount() { return f_amount; };

	private:
		QString formattedString;
		QString s_reference;
		QDate d_payable;
		QDate d_valuta;
		QDate d_paiddue;
		QString s_name;
		int i_member;
		QString s_section;
		float f_amountPaidTotal;
		float f_amountPaid;
		float f_amount;
	};
};

Q_DECLARE_METATYPE(payment::PaymentItem);
QDebug operator<<(QDebug dbg, const payment::PaymentItem &entity);
QDebug operator<<(QDebug dbg, const payment::PaymentItem *entity);

#endif // Payment_PaymentIten_H
