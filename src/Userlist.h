#ifndef Userlist_H
#define Userlist_H

#include "../ui_userlist.h"
#include <QWidget>

class Userlist : public QWidget, private Ui::UserlistForm {
Q_OBJECT
public:
	Userlist(QWidget *parent = 0);
	virtual ~Userlist();
	void loadData();
};

#endif // Userlist_H