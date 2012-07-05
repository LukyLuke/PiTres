#ifndef Contributions_H
#define Contributions_H

#include "../ui_contributions.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QSqlQueryModel>
#include <QHash>

class Contributions : public QWidget, private Ui::ContributionsForm {
Q_OBJECT

private:
	QSqlDatabase db;
	QSqlQueryModel *tableModel;
	QHash<QString, QString> sectionQif;
	
	QSqlQuery createQuery();
	void loadData();
	void createQif();

public:
	Contributions(QWidget *parent = 0);
	virtual ~Contributions();

public slots:
	void searchData();
	void exportData();
	void sendEmail();

}

#endif // Contributions_H
