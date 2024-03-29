//#include "GDemodual.h"
#include <math.h>
#include <stdio.h>


#define STANDARDNUM					200						//标准具峰的个数
#define GRATINGNUM					200						//光栅的峰的个数
#define SAMPLEPOINTNUM					10000						//采样点个数
#define DOUBLEEPSION					0.1				
#define NIHE_COUNT					10
#define XISHU_5						5



unsigned short  CurveFit(double a[], double b[], int n);



unsigned short  CalStandardPeakPos( unsigned short usStandard[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,int flag)
{
	//数组定义，将输入数组变成float型
 	 float fStandard[SAMPLEPOINTNUM]={0.0};

	//峰值信息存储
	 float fPeakPos[STANDARDNUM]={0.0};				//存储峰的位置:注意此时的位置可以是小数
	unsigned short usPeakNum = 0;				//峰的个数

	//阈值定义
	unsigned short usVTH = 1500;				//阈值电压
	unsigned short usVEXC = 500;				//峰到usVTH的距离阈值
	unsigned short usPOINTNUM = 20;				//左右点数阈值
	unsigned short usHALFWAVE = 20;				//定义半峰宽 备注随后由函数确定
	unsigned short usFILTENUM = 5;				//定义均值滤波点数
	unsigned short usPIT = flag;				//定义坑的阈值
	//float usPIT = 900.0;						//定义坑的阈值
	unsigned short usFINDPITPOINTNUM = 20;		//定义找坑是需要的点数

	//起始点和终止点定义
	unsigned short usStartPoint = 200;
	unsigned short usEndPoint = usPointNum - 200;

	//循环变量
	unsigned short usI,usJ;

	//临时变量定义
	float fTemp;
	unsigned short usTemp,usStart,usEnd,usStartTemp,usEndTemp;
	double dA[6],dB[3];							//最小二乘法来求得峰值下标

	usVTH = usVTHTemp;
	usVEXC = usVEXCTemp;

	//均值滤波，减小噪声
	usEnd = 0;

	for( usI = usEndPoint - 1;usI >= usStartPoint;usI-- )
	{
		//均值滤波--左右5个点
		//fStandard[usI] = (float)((float)usStandard[usI-5]+(float)usStandard[usI-4]+(float)usStandard[usI-3]+(float)usStandard[usI-2]+(float)usStandard[usI-1]+(float)usStandard[usI]+
		//	(float)usStandard[usI+1]+(float)usStandard[usI+2]+(float)usStandard[usI+3]+(float)usStandard[usI+4]+(float)usStandard[usI+5])/11.0;
		fStandard[usI]=usStandard[usI];
		//寻找坑

		if( (usI < (usEndPoint + usFINDPITPOINTNUM)) && (usEnd == 0) )
		{
			for( usJ = 1;usJ <= usFINDPITPOINTNUM;usJ++ )
			{
				if(fStandard[usI+usJ] > usPIT||abs(fStandard[usI+usJ]-0.0)<=DOUBLEEPSION)
				{
					break;
				}
			}			

			if( usJ >= usFINDPITPOINTNUM )
			{
				usEnd = usI;
			}
			else
			{
				usEnd=0;
			}
		}

	}

	if( usEnd != 0 )
	{
		usEndPoint = usEnd;
	}
	else
	{
		usEndPoint = usPointNum - 10;	
	}


	//寻找峰值
	usJ = usStartPoint;
	while( usJ < usEndPoint )
	{
		usJ++;
		//#if 确定上升沿
		if( (fStandard[usJ] > usVTH) && (fStandard[usJ] > fStandard[usJ-1]) && (fStandard[usJ+1] > fStandard[usJ]) && (fStandard[usJ+2] > fStandard[usJ+1]) )
		{		   
			fTemp = fStandard[usJ];
			usTemp = usJ;
			usStart = usJ;
			//#while 寻找极大值 usTemp记录了极大值的下标，usStart记录峰的起始点，usEnd记录峰的结束点
			while ( (fStandard[usJ] > usVTH) && ((fTemp-fStandard[usJ]) < usVEXC) && (usJ < usEndPoint) )
			{
				usJ++;
				if(fStandard[usJ] > fTemp)
				{
					fTemp = fStandard[usJ];
					usTemp = usJ;
				}
			}
			usEnd = usJ;


			usStartTemp = usTemp - usHALFWAVE;
			if( usStartTemp < usStart ) 
			{
				usStartTemp = usStart;
			}
			usEndTemp = usTemp + usHALFWAVE;
			if( usEndTemp > usEnd )
			{
				usEndTemp = usEnd;
			}


			//计算最小二乘法所需参数
			if( ((fTemp - fStandard[usStart]) > usVEXC) && ((usEndTemp - usStartTemp) > NIHE_COUNT)&& fTemp>fStandard[usEnd])
			{
				for( usI = 0;usI < 6;usI++ )
				{
					dA[usI] = 0.0;
				}
				for(usI = 0;usI < 3;usI++ )
				{
					dB[usI] = 0.0;
				}

				for( usI = usStartTemp;usI < usEndTemp;usI++)
				{
					dA[0] += (double)usI*(double)usI*(double)usI*(double)usI;
					dA[1] += (double)usI*(double)usI*(double)usI;
					dA[2] += (double)usI*(double)usI;	
					dA[3] += (double)usI*(double)usI;	
					dA[4] += (double)usI;
					dA[5] += (double)1.0;	

					dB[0] += (double)fStandard[usI]*(double)usI*(double)usI;
					dB[1] += (double)fStandard[usI]*(double)usI;
					dB[2] += (double)fStandard[usI];
				}
				CurveFit(dA,dB,3);
				fPeakPos[usPeakNum] = (float)((-dB[1]/dB[0])/2);
				usPeakNum++;
			}

		}
	}

	for( usI = 0;usI < usPeakNum;usI++ )
	{
		fPos[usI] = fPeakPos[usPeakNum-usI-1];
		//fPos[usI] = fPeakPos[usI];

	}

	*pusPeakNum = usPeakNum;

	return 1;
}


unsigned short  CalGratingPeakPos( unsigned short usGrating[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,double yPos[])
{
	//数组定义，将输入数组变成float型
	  float fStandard[50000]={0.0};

	//峰值信息存储	
	 float fPeakPos[GRATINGNUM]={0.0};					//存储峰的位置:注意此时的位置可以是小数
	unsigned short usPeakNum = 0;				//峰的个数

	//阈值定义
	unsigned short usVTH =  500;				//阈值电压
	unsigned short usVEXC = 500;				//峰到usVTH的距离阈值
	unsigned short usPOINTNUM = 20;				//左右点数阈值
	unsigned short usHALFWAVE = 20;				//定义半峰宽 备注随后由函数确定
	unsigned short usFILTENUM = 5;				//定义均值滤波点数

	//起始点和终止点定义
	unsigned short usStartPoint = 200;
	unsigned short usEndPoint = usPointNum - 200;

	//循环变量
	unsigned short usI,usJ;

	//临时变量定义
	float fTemp;
	unsigned short usTemp,usStart,usEnd,usStartTemp,usEndTemp;
	double dA[6],dB[3];							//最小二乘法来求得峰值下标

	static unsigned short usfirstflag = 0;

	double avg=0.0;
	double avg_sum=0.0;
	double xxxxx=0.0;

	usVTH = usVTHTemp;
	usVEXC = usVEXCTemp;


	//均值滤波，减小噪声
	//for( usI = usStartPoint;usI < usEndPoint;usI++ )
	//{
	//	fStandard[usI] = (float)((float)usGrating[usI-5]+(float)usGrating[usI-4]+(float)usGrating[usI-3]+(float)usGrating[usI-2]+(float)usGrating[usI-1]+(float)usGrating[usI]+
	//		(float)usGrating[usI+1]+(float)usGrating[usI+2]+(float)usGrating[usI+3]+(float)usGrating[usI+4]+(float)usGrating[usI+5])/11.0;
	//}

	//for(usI = 0;usI < 100;usI++)
	//{
	//	avg_sum+=usGrating[usI];
	//}
	//avg=avg_sum/100;
	//xxxxx=50000.0/avg;

	//for(usI = 0;usI < usPointNum;usI++)
	//{
	//	fStandard[usI]=xxxxx*usGrating[usI];
	//}
	for(usI=0;usI<usPointNum;usI++)
	{
		fStandard[usI]=usGrating[usI];
	}

	//寻找峰值
	usJ = usStartPoint;
	while( usJ < usEndPoint )
	{
		usJ++;
		//if 确定上升沿
		if( (fStandard[usJ] > usVTH) && (fStandard[usJ] > fStandard[usJ-1]) && (fStandard[usJ+1] > fStandard[usJ]) && (fStandard[usJ+2] > fStandard[usJ+1]) )
		{		   
			fTemp = fStandard[usJ];
			usTemp = usJ;
			usStart = usJ;
			//#while 寻找极大值 usTemp记录了极大值的下标，usStart记录峰的起始点，usEnd记录峰的结束点
			while((fStandard[usJ] > usVTH) && ((fTemp-fStandard[usJ]) < usVEXC) && (usJ < usEndPoint) )
			{
				usJ++;
				if(fStandard[usJ] > fTemp)
				{
					fTemp = fStandard[usJ];
					usTemp = usJ;
				}
			}
			usEnd = usJ;

			usStartTemp = usTemp - usHALFWAVE;
			if( usStartTemp < usStart ) 
			{
				usStartTemp = usStart;
			}
			usEndTemp = usTemp + usHALFWAVE;
			if( usEndTemp > usEnd )
			{
				usEndTemp = usEnd;
			}

			//计算最小二乘法所需参数
			if( ((fTemp - fStandard[usStart]) > usVEXC) && ((usEndTemp - usStartTemp) > NIHE_COUNT)&& fTemp>fStandard[usEnd] )
			{
				for( usI = 0;usI < 6;usI++ )
				{
					dA[usI] = 0.0;
				}
				for(usI = 0;usI < 3;usI++ )
				{
					dB[usI] = 0.0;
				}

				for( usI = usStartTemp;usI < usEndTemp;usI++)
				{
					dA[0] += (double)usI*(double)usI*(double)usI*(double)usI;
					dA[1] += (double)usI*(double)usI*(double)usI;
					dA[2] += (double)usI*(double)usI;	
					dA[3] += (double)usI*(double)usI;	
					dA[4] += (double)usI;
					dA[5] += (double)1.0;	

					dB[0] += (double)fStandard[usI]*(double)usI*(double)usI;
					dB[1] += (double)fStandard[usI]*(double)usI;
					dB[2] += (double)fStandard[usI];
				}
				CurveFit(dA,dB,3);
				fPeakPos[usPeakNum] = (float)((-dB[1]/dB[0])/2);
				yPos[usPeakNum]=dB[0]*fPeakPos[usPeakNum]*fPeakPos[usPeakNum]+dB[1]*fPeakPos[usPeakNum]+dB[2];
				usPeakNum++;
			}
		}
	}

	for( usI = 0;usI < usPeakNum;usI++ )
	{
		fPos[usI] = fPeakPos[usPeakNum-usI-1];
		//fPos[usI] = fPeakPos[usI];
	}
	*pusPeakNum = usPeakNum;

	return 1;
}

//最小二乘法曲线拟合
unsigned short  CurveFit(double a[], double b[], int n)
{
	int	i,j,k;
	double	t,t0,d;
	a[0]=1.0/a[0];
	for(i=1;i<n;i++) {
		for(j=0;j<=i-1;j++)
			if(j!=0) {
				t=0;
				for(k=0;k<=j-1;k++)
					t+=a[i*(i+1)/2+k]*a[j*(j+1)/2+k];
				a[i*(i+1)/2+j]-=t;
			}
			d=a[i*(i+1)/2+i];
			for(j=0;j<=i-1;j++) {
				t0=a[i*(i+1)/2+j];
				a[i*(i+1)/2+j]=t0*a[j*(j+1)/2+j];
				d-=t0*a[i*(i+1)/2+j];
			}
			a[i*(i+1)/2+i]=1.0/d;
	}
	for(i=1;i<n;i++) {
		t=0.0;
		for(k=0;k<=i-1;k++)
			t+=a[i*(i+1)/2+k]*b[k];
		b[i]-=t;
	}
	b[n-1]*=a[n*(n-1)/2+n-1];
	for(i=n-2;i>=0;i--) {
		t=b[i]*a[i*(i+1)/2+i];
		for(k=i+1;k<n;k++) t-=a[k*(k+1)/2+i]*b[k];
		b[i]=t;
	}

	return 0;

}


//计算光栅传感器的波长
//参数：dwavePosition 光栅波峰的下标
//		num: 标准具的波峰的个数
//      IniWave:为输入数组
//		fPosition:为标准局下标数组
double CalWaveLength(double dwavePosition, int num, double IniWave[], float fPosition[])
{
	double det;
	int point = 0,i;
	for( i = 0;i < num; i++ )
	{
		if( dwavePosition < fPosition[i] *XISHU_5)
			point = i + 1;
	}

	if(point==0)
		return IniWave[0]+(dwavePosition-fPosition[0]*XISHU_5) * ( IniWave[0] - IniWave[1] )/( fPosition[0]*XISHU_5 - fPosition[1]*XISHU_5 );
	if(point==num)
		return IniWave[num-1]+(dwavePosition-fPosition[num-1]*XISHU_5) * ( IniWave[num - 2] - IniWave[num - 1] )/( fPosition[num - 2] *XISHU_5- fPosition[num - 1]*XISHU_5 );

	det = (dwavePosition-fPosition[point-1]*XISHU_5)*(IniWave[point]-IniWave[point-1])/(fPosition[point]*XISHU_5-fPosition[point-1]*XISHU_5)+IniWave[point-1];
	return det;
}
