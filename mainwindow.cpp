#include "mainwindow.h"

#include "qextserialbase.h"
#include "posix_qextserialport.h"
#include "qcustomplot.h"
#include "thread.h"
#include "dsp.h"


#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPalette>
#include <QApplication>
#include <QTextBrowser>
#include <QTextCursor>
#include <QComboBox>
#include <QVBoxLayout>
#include <QToolBar>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QGroupBox>
#include <QMessageBox>


#define STAND_LENGTH            	50000
#define STANDARDNUM             	200

extern ushort usStandardPeakNum;
extern ushort usTemPeakNum;

bool isTransing=false;

uchar recvBuf[150000];
uint recvLength=0;
uint END_LENGTH=0;
ushort _mode=0;                      //模式选择


MainWindow::MainWindow(QWidget *parent)
:QMainWindow(parent)
{
	showMaximized();
	setWindowTitle("TDLAS");

	customPlot=new QCustomPlot();
	customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
	customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignCenter);

	customPlot_gas=new QCustomPlot();
	customPlot_gas->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);

	PB_start=new QPushButton("开始");
	PB_stop=new QPushButton("中止");
	PB_quit=new QPushButton("退出");

    	PB_stop->setEnabled(false);
	LB_signalLength=new QLabel("输入信号长度:");
	LB_firLength=new QLabel("FIR滤波器长度:");
	LB_spendTime=new QLabel("信号处理耗时(s):");
	LB_maxPos=new QLabel("最大峰值:");
	LB_status=new QLabel("状态:");
    	countLabel=new QLabel("计数:");

	LB_signalLength_value=new QLabel();
	LB_firLength_value=new QLabel();
	LB_spendTime_value=new QLabel();
	LB_maxPos_value=new QLabel();
	LB_status_value=new QLabel();
    	countLabel_value=new QLabel();

	CB_mode=new QCheckBox("Raw Mode");

	modeCombo=new QComboBox();
	modeCombo->addItem("包络");
	modeCombo->addItem("CO");
	modeCombo->addItem("CO2");
	modeCombo->addItem("C2H2");


	//工具栏
	QToolBar *toolBar=new QToolBar(this);
	toolBar->setMovable(false);
	addToolBar(Qt::TopToolBarArea,toolBar);
	toolBar->addSeparator();
	toolBar->addSeparator();
	toolBar->addWidget(modeCombo);
	toolBar->addSeparator();
	toolBar->addWidget(CB_mode);
	toolBar->addSeparator();
	toolBar->addWidget(PB_start);
	toolBar->addSeparator();
	toolBar->addWidget(PB_stop);
	toolBar->addSeparator();
	toolBar->addWidget(PB_quit);

	QGroupBox *groupBox=new QGroupBox();
	QGridLayout *grid=new QGridLayout(groupBox);
	grid->addWidget(LB_signalLength,0,0);
	grid->addWidget(LB_firLength,1,0);
	grid->addWidget(LB_spendTime,2,0);
	grid->addWidget(LB_maxPos,3,0);
	grid->addWidget(LB_status,4,0);
	grid->addWidget(LB_signalLength_value,0,1);
	grid->addWidget(LB_firLength_value,1,1);
	grid->addWidget(LB_spendTime_value,2,1);
	grid->addWidget(LB_maxPos_value,3,1);
	grid->addWidget(LB_status_value,4,1);
	grid->addWidget(countLabel,5,0);
	grid->addWidget(countLabel_value,5,1);


	QVBoxLayout *vLayout=new QVBoxLayout();
	vLayout->addWidget(customPlot,1);
	vLayout->addWidget(customPlot_gas,1);

	//布局类
	QWidget *mainW=new QWidget(this);
	setCentralWidget(mainW);
	QHBoxLayout *layout=new QHBoxLayout(mainW);
	layout->addWidget(groupBox,1);
	layout->addLayout(vLayout,5);


	connect(PB_quit,SIGNAL(clicked()),this,SLOT(PB_quit_clicked()));
	connect(PB_start,SIGNAL(clicked()),this,SLOT(PB_start_clicked()));
	connect(PB_stop,SIGNAL(clicked()),this,SLOT(PB_stop_clicked()));
    	connect(modeCombo,SIGNAL(currentIndexChanged(int)),this,SLOT(modeCombo_indexChanged(int)));


	//成员变量
	END_LENGTH=END_100003;
	_mode=MODE_VALUE_1;

	myCom=NULL;
	myCom = new Posix_QextSerialPort("/dev/ttyAMA0",QextSerialBase::Polling);
	myCom->setBaudRate(BAUD9600);
    	myCom->setDataBits(DATA_8);
	myCom->setParity(PAR_NONE);
	myCom->setStopBits(STOP_1);
    	myCom->setFlowControl(FLOW_OFF);
    	myCom->setTimeout(10);

	timer=new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(readMyCom()));
	timer->setInterval(500);

	thread=new Thread(this);
	connect(thread,SIGNAL(checkCorrect(Msg*)),this,SLOT(spi_recvOver(Msg*)));
	connect(thread,SIGNAL(checkCorrect120003(Msg*)),this,SLOT(spi_recvOver120003(Msg*)));
	isTransing=true;
	thread->start();
}

MainWindow::~MainWindow()
{
	if(NULL!=thread)
	{
		isTransing=false;
		thread->wait();
		thread->shut();
		delete thread;
		thread=NULL;
	}

	if(NULL!=timer)
	{
		delete timer;
		timer=NULL;
	}

	if(NULL!=myCom)
	{
		delete myCom;
		myCom=NULL;
	}


}

void MainWindow::readMyCom()
{

}

void MainWindow::recvOver()
{
        customPlot->legend->setVisible(false);

        thread->pDsp->raw.clear();

        vector<double> xVect,yVect;
        for(int i=0;i<5000;i++)
        {
            xVect.push_back(i*1.0);
        }

        for (int i = 0; i < 5000; i++)
        {
            thread->pDsp->raw.push_back((((uint)recvBuf[2*i] << 8) | ((uint)recvBuf[2*i+1]))*1.0);
            yVect.push_back((((uint)recvBuf[2*i] << 8) | ((uint)recvBuf[2*i+1]))*1.0);
        }

        customPlot_gas->clearGraphs();
        customPlot_gas->addGraph();
        customPlot_gas->graph(0)->setData(QVector<DSP::flt>::fromStdVector(xVect), QVector<DSP::flt>::fromStdVector(yVect));
        customPlot_gas->graph(0)->rescaleAxes();
        customPlot_gas->replot();

        if(thread->pDsp->process())
        {
            if(CB_mode->isChecked())
            {
                qDebug()<<"mode is check";

                LB_signalLength_value->setText(QString::number(thread->pDsp->laser_len));
                LB_firLength_value->setText("");
                LB_spendTime_value->setText("");
                LB_maxPos_value->setText("");

                customPlot->clearGraphs();
                customPlot->addGraph();
                customPlot->graph(0)->setData(QVector<DSP::flt>::fromStdVector(xVect), QVector<DSP::flt>::fromStdVector(thread->pDsp->raw));
                customPlot->graph(0)->rescaleAxes();
            }
            else
            {
                qDebug()<<"mode is uncheck";
                LB_signalLength_value->setText(QString::number(thread->pDsp->laser_len));
                LB_firLength_value->setText(QString::number(thread->pDsp->fir_len));
                LB_spendTime_value->setText(QString::number(thread->pDsp->elapsed));
                LB_maxPos_value->setText(QString::number(thread->pDsp->get_maximum()));

                writeWaveLengthC(".\\nongdu.csv",QString::number(thread->pDsp->get_maximum(),'f',3));

                customPlot->clearGraphs();
                customPlot->addGraph();
                customPlot->graph(0)->setData(QVector<DSP::flt>::fromStdVector(thread->pDsp->xaxis), QVector<DSP::flt>::fromStdVector(thread->pDsp->result));
                customPlot->graph(0)->rescaleAxes();
                if (thread->pDsp->peak_search())
                {
                    customPlot->addGraph();
                    customPlot->graph(1)->setPen(QPen(QColor(0,255,0)));
                    customPlot->graph(1)->setData(QVector<DSP::flt>::fromStdVector(thread->pDsp->peaks[0]), QVector<DSP::flt>::fromStdVector(thread->pDsp->peaks[1]));
                }
            }
            customPlot->replot();
        }
        else
        {
            LB_signalLength_value->setText("");
            LB_firLength_value->setText("");
            LB_spendTime_value->setText("");
            LB_maxPos_value->setText("");
            LB_status_value->setText("Error: dsp->process()");
//            LB_status_value->setStyleSheet("QLabel{color:red}");
        }
}


void MainWindow::calculator()
{
    qDebug()<<"start calculator";

    //绘标准具
    vector<double> xVect,yVect;

    for (int i = 0; i < 10000; i++)
    {
        yVect.push_back(thread->xVect[i]);
    }
    for(int i=0;i<10000;i++)
    {
        xVect.push_back(i*1.0);
    }

    customPlot_gas->clearGraphs();
    customPlot_gas->addGraph();
    customPlot_gas->graph(0)->setData(QVector<double>::fromStdVector(xVect), QVector<double>::fromStdVector(yVect));
    customPlot_gas->graph(0)->rescaleAxes();
    customPlot_gas->replot();

    //画光栅
    xVect.clear();
    for(int i=0;i<STAND_LENGTH;i++)
    {
        xVect.push_back(i*1.0);
    }

	QPen grid_pe;
    grid_pe.setColor(QColor(255,0,255));
    customPlot->clearGraphs();
    customPlot->addGraph();
    customPlot->addGraph();
    customPlot->graph(1)->setPen(grid_pe);
    customPlot->graph(1)->setName(QString("%1,%2,%3")
                                  .arg(QString::number(thread->fTemPos[usTemPeakNum-1],'f',3))
                                  .arg(QString::number(thread->yTemPos[0],'f',3))
                                  .arg(QString::number(thread->dTemWaveLength[usTemPeakNum-1],'f',3)));

    customPlot->graph(0)->setData(QVector<DSP::flt>::fromStdVector(xVect), QVector<DSP::flt>::fromStdVector(thread->yVect));
    customPlot->graph(0)->rescaleAxes();


/*	
    xVect.clear();
    yVect.clear();
    for(int i=0;i<5000;i++)
    {
        yVect.push_back(i*1.0);
    }
    if(usTemPeakNum>=1)
    {
        for(int i=0;i<5000;i++)
        {
            xVect.push_back(thread->fTemPos[usTemPeakNum-1]);
        }
    }
    customPlot->graph(1)->setData(QVector<DSP::flt>::fromStdVector(xVect), QVector<DSP::flt>::fromStdVector(yVect));
*/
    customPlot->legend->removeAt(0);
    customPlot->replot();
}

bool MainWindow::writeWaveLengthC(const QString &path, QString str)
{
    QFile file(path);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QString string=QString("%1-%2\n")
                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                .arg(str);
        file.write(string.toAscii());
        file.close();
        return true;
    }
    return false;
}

void MainWindow::PB_start_clicked()
{
    if(myCom->open(QIODevice::ReadWrite))
    {
        PB_start->setEnabled(false);
        PB_stop->setEnabled(true);

        QByteArray ay;
	//ay.append(0xFF);
	ay.append(_mode);        
	myCom->write(ay);
    }
}

void MainWindow::PB_stop_clicked()
{
        QByteArray ay;
	ay.append(0xF0);        
	myCom->write(ay);

    recvLength=0;
    if(myCom->isOpen())
        myCom->close();
    PB_start->setEnabled(true);
    PB_stop->setEnabled(false);

}

void MainWindow::PB_quit_clicked()
{
	qApp->quit();
}

void MainWindow::modeCombo_indexChanged(int index)
{
	if(!myCom->isOpen())
	{
		QMessageBox::critical(this,"提示","请打开串口");
		return ;
	}

    switch(index)
    {
    case 0:
        customPlot->xAxis->setLabel("");
        customPlot->yAxis->setLabel("");
        END_LENGTH=END_100003;
        _mode=MODE_VALUE_1;
        break;
    case 1:
        customPlot->xAxis->setLabel("时间(S)");
        customPlot->yAxis->setLabel("二次谐波系数");
            END_LENGTH=END_10003;
        _mode=MODE_VALUE_2;
        break;
    case 2:
        customPlot->xAxis->setLabel("时间(S)");
        customPlot->yAxis->setLabel("二次谐波系数");
            END_LENGTH=END_10003;
        _mode=MODE_VALUE_3;
        break;
    case 3:
        customPlot->xAxis->setLabel("时间(S)");
        customPlot->yAxis->setLabel("二次谐波系数");
        END_LENGTH=END_10003;
        _mode=MODE_VALUE_4;
        break;
    }
	QByteArray ay;
	ay.append(_mode);
	myCom->write(ay);
	modeCombo->setEnabled(false);
}

void MainWindow::spi_recvOver(Msg* pMsg)
{
	qDebug()<<"0x51:"<<QDateTime::currentDateTime().toString("hh:mm:ss");	
	for(int i=0;i<10000;i++)
	{
		recvBuf[i]=(uchar)pMsg->pData1[i];
	}

	for(int i=10000;i<10003;i++)
	{
		recvBuf[i]=(uchar)pMsg->pData3[i-10000];
	}	
	recvOver();
	modeCombo->setEnabled(true);
}

void MainWindow::spi_recvOver120003(Msg* pMsg)
{
	qDebug()<<"0x50:"<<QDateTime::currentDateTime().toString("hh:mm:ss");	
        customPlot->legend->setVisible(true);
	calculator();
	modeCombo->setEnabled(true);
}