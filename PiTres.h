#ifndef PiTres_H
#define PiTres_H

#include "ui_PiTres.h"
#include "src/Userlist.h"
#include "src/SentBills.h"

#include <QtGui/QMainWindow>
#include <QWidget>
#include <QString>

class PiTres : public QMainWindow, private Ui::MainWindow {
Q_OBJECT
private:
	QWidget *content;
	void setContent(QWidget* widget);

protected:
	void connectActions();
	void debugAction(QString sender);

public:
	PiTres(QMainWindow *parent = 0);
	virtual ~PiTres();
	void showStatusMessage(QString msg);

public slots:
	void openFile();
	void openRecently();
	void importFromLDAP();
	void importFromFile();
	void importFromClipboard();
	void showAbout();
	void showHelp(int page = 0);
	void showUsers();
	void showSentBills();
	void showCreateBills();
};

#endif // PiTres_H
