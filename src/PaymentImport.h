#ifndef PaymentImport_H
#define PaymentImport_H

#include "../ui_paymentimport.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QTimerEvent>

// For QIF-Spec see http://www.respmech.com/mym2qifw/qif_new.htm

class PaymentImport : public QWidget, private Ui::PaymentImportForm {
Q_OBJECT
private:
	QString INVOICE_PAID_STATE;
	QSqlDatabase db;
	int searchTimer;
	
	void openDatabase();

public:
	PaymentImport(QWidget *parent = 0);
	virtual ~PaymentImport();
	
public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void continueImport();
	void paySelectedInvoice();
	void exportQifLiabilities();
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // PaymentImport_H