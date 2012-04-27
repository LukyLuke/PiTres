#ifndef Helper_Formatter_H 
#define Helper_Formatter_H 

#include <QString>
#include <QLocale>

class Formatter {
public:
	static QString telephone(QString number);
	static QString telephone(QString number, QLocale::Country country);
};

#endif // Helper_Formatter_H 
