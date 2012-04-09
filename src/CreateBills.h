#ifndef CreateBills_H
#define CreateBills_H

#include "../ui_createbills.h"

#include <QWidget>

class CreateBills : public QWidget, private Ui::CreateBillsForm {
Q_OBJECT
public:
	CreateBills(QWidget *parent = 0);
	virtual ~CreateBills();
	void loadData();
};

#endif // CreateBills_H