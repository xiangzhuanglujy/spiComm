#include "thread.h"
#include "bcm2835.h"
#include "dsp.h"

#include <QDebug>
#include <QDateTime>


extern "C"
{
#include "GDemodual.h"
}


#define PIN 				RPI_GPIO_P1_15
//接收缓冲区



extern bool isTransing;
extern ushort _mode;
ushort grating[STAND_LENGTH];
ushort standing[10000];
double dIniWave[40];
ushort usStandardPeakNum=0;
ushort usTemPeakNum=0;


Thread::Thread(QObject *parent)
:QThread(parent)
{
        if (!bcm2835_init())
        {
            std::cout << "Initialize bcm2835 failed." << std::endl;
            return;
        }

        bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(PIN, BCM2835_GPIO_PUD_DOWN);
        bcm2835_spi_begin();
        bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
        bcm2835_spi_setDataMode(BCM2835_SPI_MODE2);                   // The default
        bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); // The default
        bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
        bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

	//算术类
	pDsp=new DSP();

	for(int i=0;i<40;i++)
	{
		dIniWave[i]=(1550-i)*1.0;
	}
}

void Thread::run()
{
	qDebug()<<"thread enter";
	char recv[100000];	
	char recv1[20000];
	char recv2[3];
	Msg msg;
	msg.pData1=recv;
	msg.pData2=recv1;
	msg.pData3=recv2;


	while(isTransing)
	{	
		qDebug()<<"run enter";
		if(!bcm2835_gpio_lev(PIN))
		{
			bcm2835_spi_transfern(recv, 100000);
			//msleep(1);
			bcm2835_spi_transfern(recv1, 20000);
			//msleep(1);
			bcm2835_spi_transfern(recv2, 3);
			//msleep(1);
			qDebug()<<"recv len="<<(uchar)recv2[0]<<(uchar)recv2[1]<<(uchar)recv2[2];
			

			if(_mode==0x50&&(uchar)recv2[0]==0x50&&(uchar)recv2[1]==0xA0&&(uchar)recv2[2]==0xA0)
			{				
				qDebug()<<"thread emit:"<<QDateTime::currentDateTime().toString("hh:mm:ss");	

				xVect.clear();
				yVect.clear();				

				for (int i = 0; i < 50000; i++)
				{
				    yVect.push_back((((uint)recv[2*i] << 8)|((uint)recv[2*i+1]))*1.0);
				}

				for (int i = 0; i < 10000; i++)
				{
				    xVect.push_back((((uint)recv1[2*i] << 8)|((uint)recv1[2*i]))*1.0);
				}


				if(pDsp->processX(xVect))
				{

				}

				qDebug()<<"B:"<<QDateTime::currentDateTime().toString("hh:mm:ss");
				if(pDsp->process(yVect))
				{

				}

				qDebug()<<"C:"<<QDateTime::currentDateTime().toString("hh:mm:ss");
					

			    //分离标准具和光栅
			    for (int i = 0; i < STAND_LENGTH; i++)
			    {
				grating[i]=yVect[i];
			    }
			    for (int i = 0; i < 10000; i++)
			    {
				standing[i]=xVect[i];
			    }


			    //计算波长

			CalStandardPeakPos(standing,10000,fpos,&usStandardPeakNum,9000,200,2000);
			CalGratingPeakPos(grating,STAND_LENGTH,fTemPos,&usTemPeakNum,1000,500,yTemPos);

			for(int i = 0;i < usTemPeakNum;i++ )
			{
			dTemWaveLength[i] = CalWaveLength( fTemPos[i],usStandardPeakNum,dIniWave,fpos);
			}

	qDebug()<<"D:"<<QDateTime::currentDateTime().toString("hh:mm:ss");
				emit checkCorrect120003(&msg);
			}
			else if(_mode!=0x50&&(uchar)recv[10000]==_mode&&(uchar)recv[10001]==0xA0&&(uchar)recv[10002]==0xA0)
			{
				emit checkCorrect(&msg);
			}
		}
                msleep(200);
	}
}

void Thread::shut()
{
	bcm2835_spi_end();
	bcm2835_close();

	if(NULL!=pDsp)
	{
		delete pDsp;
		pDsp=NULL;
	}
}


