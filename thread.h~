#ifndef THREAD_H_
#define THREAD_H_

#include <QThread>
#include <iostream>
#include <vector>

#include "define.h"


#define STAND_LENGTH			50000
#define STANDARDNUM             	200


using namespace std;


class DSP;


class Thread:public QThread
{
	Q_OBJECT
public:
	explicit Thread(QObject *parent=0);

	void shut();
	//算法类
	DSP *pDsp;
	vector<double> yVect;
	vector<double> xVect;
	float  fpos[STANDARDNUM];
	float  fTemPos[STANDARDNUM];
	double  yTemPos[STANDARDNUM];
	double dTemWaveLength[STANDARDNUM];

signals:
	void checkCorrect(Msg*);
	void checkCorrect120003(Msg*);

protected:
	void run();

private:

};


#endif
