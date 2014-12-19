#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	MainWindow window;
	window.resize(1100, 1000);
	window.show();

	return app.exec();
}
