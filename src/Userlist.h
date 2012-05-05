#ifndef Userlist_H
#define Userlist_H

#include "../ui_userlist.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QTimerEvent>

class Userlist : public QWidget, private Ui::UserlistForm {
Q_OBJECT
private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	int searchTimer;
	
	void openDatabase();
	void loadSections();
	QSqlQuery createQuery();

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

protected:
	void timerEvent(QTimerEvent *event);
};

#endif // Userlist_H