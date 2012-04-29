#ifndef SentBills_H
#define SentBills_H

#include "../ui_sentbills.h"

#include <QWidget>
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
	
	void openDatabase();

public:
	SentBills(QWidget *parent = 0);
	virtual ~SentBills();
	void loadData();

public slots:
	void searchData();
	void searchDataTimeout(QString data);

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // SentBills_H