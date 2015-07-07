#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QTextCodec>
#include <QCleanlooksStyle>
#include <QMetaType>
//#include <QFile>

#include "mainwindow.h"


int main(int argc,char *argv[])
{
	QApplication app(argc,argv);
	//app.setStyle(new QCleanlooksStyle);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	qRegisterMetaType<Msg>("Msg");
/*
	QFile file("qtstyle.qss");
	if(file.open(QIODevice::ReadOnly))
	{
		app.setStyleSheet(file.readAll());
		file.close();
	}
*/

	MainWindow window;
/*
	int screenWidth, screenHeight;
	QDesktopWidget *desktop = QApplication::desktop();
	screenWidth = desktop->width();
	screenHeight = desktop->height();
	window.resize(screenWidth, screenHeight);
	window.move(0, 0);
	window.setWindowFlags(Qt::CustomizeWindowHint);
	window.show();
*/
	window.resize(300,200);
	window.show();
	return app.exec();
}
