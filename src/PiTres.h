#ifndef PiTres_H
#define PiTres_H

#include "../ui_PiTres.h"
#include "../ui_settings.h"

#include <QtGui/QMainWindow>
#include <QWidget>
#include <QDialog>
#include <QString>

class PiTres : public QMainWindow, private Ui::MainWindow {
Q_OBJECT
private:
	QDialog *settingsDialog;
	Ui::settingsForm settingsForm;
	
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
	void showImportPayments();
	void showSettings();
	void doSaveSettings();
};

#endif // PiTres_H
