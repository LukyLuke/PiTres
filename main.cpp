
#include "src/PiTres.h"
#include <QtGui/QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QLocale>
#include <QTranslator>
#include <QTextCodec>
#include <QDebug>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QSettings settings;
	
	// Application Settings
	QCoreApplication::setOrganizationName("Piratenpartei");
	QCoreApplication::setOrganizationDomain("piratenpartei.ch");
	QCoreApplication::setApplicationName("PiTres");
	QCoreApplication::setApplicationVersion("0.1.0");
	
	// Some default Settings
	QLocale::setDefault(settings.value("core/locale", QLocale(QLocale::SwissGerman, QLocale::Switzerland)).toLocale());
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
	QString lang = settings.value("core/language", QLocale::system().name()).toString();
	
	// Translations
	QTranslator translator;
	if (!translator.load(QString("pitres_") + lang, "./i10n/")) {
		qDebug() << "Translation not found for " + lang;
	}
	app.installTranslator(&translator);
	
	PiTres pitres;
	pitres.show();
	return app.exec();
}
