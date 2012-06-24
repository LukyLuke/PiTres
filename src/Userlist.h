#ifndef Userlist_H
#define Userlist_H

#include "../ui_userlist.h"
#include "../ui_dateform.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QAction>
#include <QSet>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QDialog>
#include <QTimerEvent>

class Userlist : public QWidget, private Ui::UserlistForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	QAction *actionManualPayment;
	QAction *actionEditDueDate;
	
	QDialog *editDateDialog;
	Ui::dateForm dateForm;
	
	void loadSections();
	QSqlQuery createQuery();
	void createContextMenu();
	int getFirstSelectedUid();

public:
	Userlist(QWidget *parent = 0);
	virtual ~Userlist();
	void loadData();

public slots:
	void searchData();
	void searchDataTimeout(QString data);
	void filterSection(QString section);
	void showDetails(int index);
	void showDetails(QModelIndex index);
	void showTableContextMenu(const QPoint &point);
	void exportData();
	void payMemberfeeCash();
	void showMemberDueAdjust();
	void adjustMemberDueDate();

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // Userlist_H