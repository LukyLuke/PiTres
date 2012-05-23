######################################################################
# Automatically generated by qmake (2.01a) Sat Mar 17 08:23:12 2012
######################################################################

TEMPLATE = app
TARGET = PiTres
DEPENDPATH += .
INCLUDEPATH += . src

# Main sources
HEADERS += src/PiTres.h src/Userlist.h src/SentBills.h src/PaymentImport.h src/LDAPImport.h src/InvoiceWizard.h src/PaymentWizard.h
SOURCES += main.cpp src/PiTres.cpp src/Userlist.cpp src/SentBills.cpp src/PaymentImport.cpp src/LDAPImport.cpp src/InvoiceWizard.cpp src/PaymentWizard.cpp

# Data Objects
HEADERS += src/data/Person.h src/data/Invoice.h src/data/Reminder.h
SOURCES += src/data/Person.cpp src/data/Invoice.cpp src/data/Reminder.cpp

# Delegates
#HEADERS += src/delegate/.h
#SOURCES += src/delegate/.cpp

# Helper Classes and Functions
HEADERS += src/helper/Formatter.h src/helper/XmlPdf.h src/helper/PdfElement.h src/helper/Smtp.h
SOURCES += src/helper/Formatter.cpp src/helper/XmlPdf.cpp src/helper/PdfElement.cpp src/helper/Smtp.cpp

# Config
CONFIG += qt thread sql
RESOURCES += PiTres.qrc
QT = core gui network sql xml
FORMS = forms/PiTres.ui forms/userlist.ui forms/sentbills.ui forms/paymentimport.ui forms/LDAPImport.ui \
        forms/adjustpaiddate.ui forms/fromtodates.ui forms/settings.ui forms/dateform.ui forms/invoicewizard.ui \
        forms/payment.ui
OUT_PWD = build

# Add special Platform-Dependant libraries and sources
win32 {
	CONFIG += windows
	LIBS += -lldap -llber
}
unix {
	LIBS += -lldap -llber
}
macx {

}

# Include custom build
include(build.pri)

# enable debuging
!isEmpty(DEBUG) {
	CONFIG += debug
	win32:debug {
		CONFIG += console
	}
	macx:debug {
	}
}

# Checking for SqLite
!isEmpty(USE_SQLITE) {
	CONFIG += sqlite3
	HEADERS += 
	SOURCES += 
}
isEmpty(USE_SQLITE) {
	warning("SqLite3 is needed if you want store data locally")
}




