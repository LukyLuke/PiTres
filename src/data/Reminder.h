#ifndef Data_Reminder_H
#define Data_Reminder_H

#include "Invoice.h"
#include "../helper/XmlPdf.h"

class Reminder : public Invoice {
public:
	Reminder(QObject *parent = 0);
	virtual ~Reminder();
	XmlPdf *createPdf();
};

#endif // Data_Reminder_H 
