#ifndef PiTres_H
#define PiTres_H

#include "../ui_PiTres.h"
#include "../ui_settings.h"
#include "../ui_invoicewizard.h"

#include <QtGui/QMainWindow>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QSqlDatabase>

class PiTres : public QMainWindow, private Ui::MainWindow {
Q_OBJECT
private:
	QDialog *settingsDialog;
	Ui::settingsForm settingsForm;
	QSqlDatabase db;
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
	void importFromLDAP();
	void showAbout();
	void showHelp(int page = 0);
	void showUsers();
	void showSentBills();
	void showImportPayments();
	void showInvoiceWizard();
	void showSettings();
	void doSaveSettings();
	void showInvoiceFileDialog();
	void showReminderFileDialog();
	void showReceiptFileDialog();
	void showContributions();
};

#endif // PiTres_H
