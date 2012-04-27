
#include "Formatter.h"

#include <QString>
#include <QRegExp>
#include <QLocale>

QString Formatter::telephone(QString number) {
	return Formatter::telephone(number, QLocale::Switzerland);
}

QString Formatter::telephone(QString number, QLocale::Country country) {
	number = number.replace(QRegExp("[^0-9]+"), "").trimmed();
	if (!number.startsWith("0")) {
		number.prepend("++");
	}
	/*switch (country) {
		default:  break;
	}*/
	return number;
}

