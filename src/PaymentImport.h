#ifndef PaymentImport_H
#define PaymentImport_H

#include "../ui_paymentimport.h"
#include "data/Person.h"

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
	QSqlDatabase db;
	int searchTimer;
	QString listString;
	QString reString;

public:
	PaymentImport(QWidget *parent = 0);
	virtual ~PaymentImport();
	
public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void continuePayment();
	void addSelectedInvoice();
	void invoiceSelected(QListWidgetItem *item);
	
protected:
	void timerEvent(QTimerEvent *event);
};

#endif // PaymentImport_H