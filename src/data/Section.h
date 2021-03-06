/*
	A Section, a Sub-Department of the main pirate party
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

#ifndef Data_Section_H
#define Data_Section_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QDate>
#include <QFile>
#include <QSqlDatabase>

class Section : public QObject {
Q_OBJECT
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
Q_PROPERTY(float amount READ amount WRITE setAmount NOTIFY amountChanged)
Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)
Q_PROPERTY(int treasurer READ treasurer WRITE setTreasurer NOTIFY treasurerChanged)
Q_PROPERTY(QDate founded READ founded WRITE setFoundingDate NOTIFY foundedChanged)
Q_PROPERTY(QString invoiceLogo READ invoiceLogo WRITE setInvoiceLogo NOTIFY invoiceLogoChanged)
Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)

Q_ENUMS(AmountType)

public:
	Section(QObject* parent = 0);
	Section(const QString name, QObject* parent = 0);
	virtual ~Section();
	void load(const QString name);
	void save();
	QList<Section *> children();
	Section *parent();
	bool logoIsFile();
	static void createTables();
	static void getNameParentHash(QHash<QString, QString> *hash);
	static void getSectionList(QList<QString> *list);
	static QString getSectionName(const QString key);
	static QString getSectionDescription(const QString key);
	
	// enums
	enum AmountType { AmountPercent=0, AmountMoney=1 };
	
	// Setter
	void setParent(QString parent);
	void setName(QString name);
	void setAmount(float amount);
	void setDescription(QString description);
	void setAddress(QString address);
	void setAccount(QString account);
	void setTreasurer(int treasurer);
	void setFoundingDate(QDate date);
	void setInvoiceLogo(QString logo);
	void setInvoiceText(QString text, QString language);
	void setEmail(QString email);
	
	// Getter
	bool loaded() { return _loaded; };
	QString name() const { return s_name; }
	float amount() const { return f_amount; }
	QString description() const { return s_description; }
	QString address() const { return s_address; }
	QString account() const { return s_account; }
	int treasurer() const { return i_treasurer; }
	QDate founded() const { return d_founded; }
	QString invoiceLogo() const { return s_invoiceLogo; }
	QString invoiceText(QString language) const { return m_invoiceLanguage.value(language, ""); }
	QString emailDB() const { return s_email; };
	QString email();
	
signals:
	void parentChanged(QString);
	void nameChanged(QString);
	void amountChanged(float);
	void descriptionChanged(QString);
	void addressChanged(QString);
	void accountChanged(QString);
	void treasurerChanged(int);
	void foundedChanged(QDate);
	void invoiceLogoChanged(QString);
	void invoiceTextChanged(QString, QString);
	void emailChanged(QString);
	
private:
	QSqlDatabase db;
	bool _loaded;
	QString s_parent;
	QString s_name;
	float f_amount;
	QString s_description;
	QString s_address;
	QString s_account;
	int i_treasurer;
	QDate d_founded;
	QString s_invoiceLogo;
	QMap<QString, QString> m_invoiceLanguage;
	QString s_email;
	static QHash<QString, QString> m_namecache;
	static QHash<QString, QString> m_desccache;
	
	void clear();
};

#endif //Data_Section_H
