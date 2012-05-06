
#include "Reminder.h"

#include <QSettings>
#include <QVariant>
#include <QString>

Reminder::Reminder(QObject *parent) : Invoice(parent) {
}

Reminder::~Reminder() {}

XmlPdf Reminder::createPdf() {
	QSettings settings;
	XmlPdf pdf;
	pdf.loadTemplate( settings.value("pdf/reminder_template", "data/reminder.xml").toString() );
	return pdf;
}