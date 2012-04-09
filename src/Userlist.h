#ifndef Userlist_H
#define Userlist_H

#include "../ui_userlist.h"

#include <QWidget>
#include <QtSql>

class Userlist : public QWidget, private Ui::UserlistForm {
Q_OBJECT
private:
	QSqlDatabase db;

public:
	Userlist(QWidget *parent = 0);
	virtual ~Userlist();
	void loadData();
};

#endif // Userlist_H