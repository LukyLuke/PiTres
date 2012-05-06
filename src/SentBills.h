#ifndef SentBills_H
#define SentBills_H

#include "../ui_sentbills.h"
#include "../ui_adjustpaiddate.h"
#include "../ui_fromtodates.h"
#include "data/Reminder.h"

#include <QWidget>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QTimerEvent>
#include <QPoint>
#include <QMenu>
#include <QSet>
#include <QAction>

class SentBills : public QWidget, private Ui::SentBillsForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	QAction *actionSendReminder;
	QAction *actionPrintReminder;
	
	QDialog *adjustDialog;
	QDialog *fromtoDialog;
	Ui::adjustPaidDateForm form;
	Ui::fromToDatesForm fromto;
	
	void openDatabase();
	QSqlQuery createQuery();
	void createContextMenu();
	QSet<int> getSelectedRows();

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
	void showTableContextMenu(const QPoint &point);
	void sendNewReminder();
	void printNewReminder();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // SentBills_H