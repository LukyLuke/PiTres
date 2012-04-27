#ifndef Data_Invoice_H
#define Data_Invoice_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QList>
#include <QSqlDatabase>

class Invoice : public QObject {
Q_OBJECT
Q_PROPERTY(int memberUid READ memberUid WRITE setMemberUid NOTIFY memberUidChanged)
Q_PROPERTY(QString reference READ reference WRITE setReference NOTIFY referenceChanged)
Q_PROPERTY(QDate issueDate READ issueDate WRITE setIssueDate NOTIFY issueDateChanged)
Q_PROPERTY(QDate payableDue READ payableDue WRITE setPayableDue NOTIFY payableDueChanged)
Q_PROPERTY(QDate paidDate READ paidDate WRITE setPaidDate NOTIFY paidDateChanged)
Q_PROPERTY(float amount READ amount WRITE setAmount NOTIFY amountChanged)
Q_PROPERTY(float amountPaid READ amountPaid WRITE setAmountPaid NOTIFY amountPaidChanged)
Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
Q_PROPERTY(int reminded READ reminded WRITE setReminded NOTIFY remindedChanged)

// This is the owner
Q_PROPERTY(QString addressPrefix READ addressPrefix WRITE setAddressPrefix NOTIFY addressPrefixChanged)
Q_PROPERTY(QString addressCompany READ addressCompany WRITE setAddressCompany NOTIFY addressCompanyChanged)
Q_PROPERTY(QString addressName READ addressName WRITE setAddressName NOTIFY addressNameChanged)
Q_PROPERTY(QString addressStreet1 READ addressStreet1 WRITE setAddressStreet1 NOTIFY addressStreet1Changed)
Q_PROPERTY(QString addressStreet2 READ addressStreet2 WRITE setAddressStreet2 NOTIFY addressStreet2Changed)
Q_PROPERTY(QString addressCity READ addressCity WRITE setAddressCity NOTIFY addressCityChanged)
Q_PROPERTY(QString addressEmail READ addressEmail WRITE setAddressEmail NOTIFY addressEmailChanged)
Q_PROPERTY(QString forSection READ forSection WRITE setForSection NOTIFY forSectionChanged)

Q_ENUMS(State)

public:
	Invoice(QObject *parent = 0);
	virtual ~Invoice();
	void save(QSqlDatabase db);
	void clear();
	void setIsLoaded(bool loaded);
	void loadLast(QSqlDatabase db, int member);
	static QList<Invoice *> getInvoicesForMember(QSqlDatabase db, int member);
	static void createTables(QSqlDatabase db);
	
	// InvoiceState in old DB: o_pen, c_anceled, p_aid, u_nknown
	enum State { StateOpen=0, StateCanceled=1, StatePaid=2, StateUnknown=3 };
	
	// Setter
	void setMemberUid(int memberUid);
	void setReference(QString reference);
	void setIssueDate(QDate issueDate);
	void setPayableDue(QDate payableDue);
	void setPaidDate(QDate paidDate);
	void setAmount(float amount);
	void setAmountPaid(float amountPaid);
	void setState(State state);
	void setReminded(int reminded);
	void setAddressPrefix(QString addressPrefix);
	void setAddressCompany(QString addressCompany);
	void setAddressName(QString addressName);
	void setAddressStreet1(QString addressStreet1);
	void setAddressStreet2(QString addressStreet2);
	void setAddressCity(QString addressCity);
	void setAddressEmail(QString addressEmail);
	void setForSection(QString forSection);
	
	// Getter
	int memberUid() const { return i_memberUid; };
	QString reference() const { return s_reference; };
	QDate issueDate() const { return d_issueDate; };
	QDate payableDue() const { return d_payableDue; };
	QDate paidDate() const { return d_paidDate; };
	float amount() const { return f_amount; };
	float amountPaid() const { return f_amountPaid; };
	State state() const { return m_state; };
	int reminded() const { return i_reminded; };
	QString addressPrefix() const { return s_addressPrefix; };
	QString addressCompany() const { return s_addressCompany; };
	QString addressName() const { return s_addressName; };
	QString addressStreet1() const { return s_addressStreet1; };
	QString addressStreet2() const { return s_addressStreet2; };
	QString addressCity() const { return s_addressCity; };
	QString addressEmail() const { return s_addressEmail; };
	QString forSection() const { return s_forSection; };
	
signals:
	void memberUidChanged(int);
	void referenceChanged(QString);
	void issueDateChanged(QDate);
	void payableDueChanged(QDate);
	void paidDateChanged(QDate);
	void amountChanged(float);
	void amountPaidChanged(float);
	void stateChanged(State);
	void remindedChanged(int);
	void addressPrefixChanged(QString);
	void addressCompanyChanged(QString);
	void addressNameChanged(QString);
	void addressStreet1Changed(QString);
	void addressStreet2Changed(QString);
	void addressCityChanged(QString);
	void addressEmailChanged(QString);
	void forSectionChanged(QString);
	
private:
	bool _loaded;
	int i_memberUid;
	QString s_reference;
	QDate d_issueDate;
	QDate d_payableDue;
	QDate d_paidDate;
	float f_amount;
	float f_amountPaid;
	State m_state;
	int i_reminded;
	QString s_addressPrefix;
	QString s_addressCompany;
	QString s_addressName;
	QString s_addressStreet1;
	QString s_addressStreet2;
	QString s_addressCity;
	QString s_addressEmail;
	QString s_forSection;
};

#endif // Data_Invoice_H