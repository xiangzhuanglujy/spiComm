#ifndef _GDEMODUAL_H_
#define _GDEMODUAL_H_

unsigned short  CalStandardPeakPos( unsigned short usStandard[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,int flag);

unsigned short  CalGratingPeakPos( unsigned short usGrating[],unsigned short usPointNum,float fPos[],unsigned short* pusPeakNum,unsigned short usVTHTemp,unsigned short usVEXCTemp,double yPos[]);

double  CalWaveLength(double dwavePosition, int num, double IniWave[], float fPosition[]);

#endif