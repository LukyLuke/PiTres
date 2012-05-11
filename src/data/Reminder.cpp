
#include "Reminder.h"

#include <QSettings>
#include <QVariant>
#include <QString>
#include <QDate>

Reminder::Reminder(QObject *parent) : Invoice(parent) {
}

Reminder::~Reminder() {}

XmlPdf *Reminder::createPdf() {
	QSettings settings;
	XmlPdf *pdf = new XmlPdf;
	pdf->loadTemplate( settings.value("pdf/reminder_template", "data/reminder.xml").toString() );
	
	pdf->setVar("pp_country", settings.value("pdf/var_pp_country", "CH").toString());
	pdf->setVar("pp_zip", settings.value("pdf/var_pp_zip", "8500").toString());
	pdf->setVar("pp_city", settings.value("pdf/var_pp_city", "Frauenfeld").toString());
	pdf->setVar("print_city", settings.value("pdf/var_print_city", "Frauenfeld").toString());
	pdf->setVar("print_date", QDate::currentDate().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("print_year", QDate::currentDate().toString("yyyy"));
	
	pdf->setVar("invoice_reference", reference());
	pdf->setVar("invoice_number", reference());
	pdf->setVar("invoice_date", issueDate().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("invoice_payable_due", payableDue().toString( settings.value("pdf/date_format", "dd.MM.yyyy").toString() ));
	pdf->setVar("invoice_amount", QString("%1").arg(amount()));
	pdf->setVar("invoice_esr", "");
	
	pdf->setVar("member_number", QString::number(memberUid()));
	pdf->setVar("member_prefix", addressPrefix());
	pdf->setVar("member_company", addressCompany());
	pdf->setVar("member_name", addressName());
	pdf->setVar("member_street", addressStreet1());
	pdf->setVar("member_street2", addressStreet2());
	pdf->setVar("member_city", addressCity());
	pdf->setVar("member_country", addressCountry());
	pdf->setVar("member_email", addressEmail());
	pdf->setVar("member_section", forSection());
	
	return pdf;
}