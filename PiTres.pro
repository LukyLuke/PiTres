######################################################################
# Automatically generated by qmake (2.01a) Sat Mar 17 08:23:12 2012
######################################################################

# Enable debug?
DEBUG = True

# Uncomment to use the FIO-Calculation method where each section may make a membership fee recommendation
DEFINES += "FIO"

# Comment out to send EMails
DEFINES += "SMTP_DEBUG"

# Comment out to save EMails in a debug folder - each mail wi get a folder with all needed data inside.
# This takes affect only when you are in SMTP_DEBUG.
DEFINES += "SMTP_SAVE_DEBUG"

# Global parameters
TEMPLATE = app
TARGET = PiTres
DEPENDPATH += .
INCLUDEPATH += . src

# Main sources
HEADERS += src/PiTres.h src/Userlist.h src/SentBills.h src/PaymentImport.h src/LDAPImport.h src/InvoiceWizard.h \
           src/PaymentWizard.h src/Contributions.h src/BudgetView.h src/SectionEdit.h src/Donations.h src/DonationWizard.h src/Statistics.h
SOURCES += main.cpp src/PiTres.cpp src/Userlist.cpp src/SentBills.cpp src/PaymentImport.cpp src/LDAPImport.cpp src/InvoiceWizard.cpp \
           src/PaymentWizard.cpp src/Contributions.cpp src/BudgetView.cpp src/SectionEdit.cpp src/Donations.cpp src/DonationWizard.cpp src/Statistics.cpp

# Data Objects
HEADERS += src/data/Person.h src/data/Invoice.h src/data/Reminder.h src/data/Section.h src/data/BudgetEntity.h src/data/Donation.h
SOURCES += src/data/Person.cpp src/data/Invoice.cpp src/data/Reminder.cpp src/data/Section.cpp src/data/BudgetEntity.cpp src/data/Donation.cpp

# Models
HEADERS += src/model/budget_BudgetEntityModel.h src/model/budget_TreeModel.h src/model/payment_PaymentImportModel.h
SOURCES += src/model/budget_BudgetEntityModel.cpp src/model/budget_TreeModel.cpp src/model/payment_PaymentImportModel.cpp

# Delegates
HEADERS += src/delegate/budget_BudgetEntityDelegate.h src/delegate/budget_TreeItemDelegate.h
SOURCES += src/delegate/budget_BudgetEntityDelegate.cpp src/delegate/budget_TreeItemDelegate.cpp

# Model-Items
HEADERS += src/item/budget_TreeItem.h src/item/payment_PaymentItem.h
SOURCES += src/item/budget_TreeItem.cpp src/item/payment_PaymentItem.cpp

# Helper Classes and Functions
HEADERS += src/helper/Formatter.h src/helper/XmlPdf.h src/helper/PdfElement.h src/helper/Smtp.h src/helper/FIOCalc.h src/helper/CatchKeyPress.h
SOURCES += src/helper/Formatter.cpp src/helper/XmlPdf.cpp src/helper/PdfElement.cpp src/helper/Smtp.cpp src/helper/FIOCalc.cpp src/helper/CatchKeyPress.cpp

# Config
CONFIG += qt thread sql
RESOURCES += PiTres.qrc
QT = core gui network sql xml svg
FORMS = forms/PiTres.ui forms/userlist.ui forms/sentbills.ui forms/paymentimport.ui forms/LDAPImport.ui \
        forms/fromtodates.ui forms/settings.ui forms/dateform.ui forms/invoicewizard.ui \
        forms/payment.ui forms/invoiceedit.ui forms/contributions.ui forms/budget.ui forms/editsections.ui \
        forms/donations.ui forms/donation_import.ui forms/statistics.ui forms/invoiceexportcsv.ui
OUT_PWD = build

# Translations
TRANSLATIONS = i18n/pitres_de.ts \
               i18n/pitres_fr.ts \
               i18n/pitres_it.ts

# Add special Platform-Dependant libraries and sources
win32 {
	CONFIG += windows
	#LIBS += -lldap -llber -lmagic
  WIN32 = yes
}
unix {
	LIBS += -lldap -llber -lmagic
}
macx {

}

# Include custom build
include(build.pri)

# enable debuging
!isEmpty(DEBUG) {
	DEFINES += "DEBUG"
	CONFIG += debug warn_on
	CONFIG -= release
	
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




