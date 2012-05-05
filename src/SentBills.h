#ifndef SentBills_H
#define SentBills_H

#include "../ui_sentbills.h"
#include "../ui_adjustpaiddate.h"
#include "../ui_fromtodates.h"

#include <QWidget>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QTimerEvent>

class SentBills : public QWidget, private Ui::SentBillsForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	
	QDialog *adjustDialog;
	Ui::adjustPaidDateForm form;
	
	QDialog *fromtoDialog;
	Ui::fromToDatesForm fromto;
	
	void openDatabase();
	QSqlQuery createQuery();

public:
	SentBills(QWidget *parent = 0);
	virtual ~SentBills();
	void loadData();

public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void syncUserPaidDate();
	void doAdjustDates();
	void exportQifAssets();
	void doExportQifAssets();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // SentBills_H