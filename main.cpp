
#include <QtGui/QApplication>
#include "PiTres.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	PiTres foo;
	foo.show();
	return app.exec();
}
