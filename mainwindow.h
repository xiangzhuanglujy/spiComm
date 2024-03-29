#ifndef MAINWINDOW_H_
#define MIANWINDOW_H_

#include <QMainWindow>
#include <vector>

#include "define.h"

using namespace std;

class QLabel;
class Posix_QextSerialPort;
class QTimer;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QTextBrowser;
class QComboBox;
class QTextBrowser;
class QCustomPlot;
class Thread;


class MainWindow:public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent=0);
	~MainWindow();

	void recvOver();
	void calculator();
	bool writeWaveLengthC(const QString &path, QString str);


	enum ENDLENGTH
	{
		END_100003=120003,
		END_10003=10003
	};

	enum MODEVALUE
	{
		MODE_VALUE_1=0x50,
		MODE_VALUE_2=0x51,
		MODE_VALUE_3=0x52,
		MODE_VALUE_4=0x53
	};

public slots:
	void readMyCom();

	void PB_start_clicked();
	void PB_stop_clicked();
	void PB_quit_clicked();
    	void modeCombo_indexChanged(int);
	void spi_recvOver(Msg*);
	void spi_recvOver120003(Msg*);

private:
	Posix_QextSerialPort *myCom;

	QTimer *timer;

	QLabel *LB_signalLength;
	QLabel *LB_firLength;	
	QLabel *LB_spendTime;
	QLabel *LB_maxPos;
	QLabel *LB_status;
    	QLabel *countLabel;

	QLabel *LB_signalLength_value;
	QLabel *LB_firLength_value;
	QLabel *LB_spendTime_value;
	QLabel *LB_maxPos_value;
	QLabel *LB_status_value;
    	QLabel *countLabel_value;

	QCheckBox *CB_mode;

	QPushButton *PB_start;
	QPushButton *PB_stop;
	QPushButton *PB_quit;

	QCustomPlot *customPlot;
    	QCustomPlot *customPlot_gas;

    	QComboBox *modeCombo;

	Thread *thread;
};


#endif
