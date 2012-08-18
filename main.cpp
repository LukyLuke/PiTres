/*
	The main start and initialization
	Copyright (C) 2012  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	QCoreApplication::setOrganizationName("PirateParty");
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
