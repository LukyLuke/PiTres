#ifndef SentBills_H
#define SentBills_H

#include "../ui_sentbills.h"

#include <QWidget>

class SentBills : public QWidget, private Ui::SentBillsForm {
Q_OBJECT
public:
	SentBills(QWidget *parent = 0);
	virtual ~SentBills();
	void loadData();
};

#endif // SentBills_H