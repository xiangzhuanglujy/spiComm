#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <ctime>
#include "dsp.h"
//#include "spi.h"
#include <QDebug>
#include <QDateTime>

DSP::DSP(const string input, const string fir, const string output, const flt fsin, const flt fsaw, const flt fsample) :
file_input(input), file_fir(fir), file_output(output),
laser_len(0), fir_len(0), elapsed(0.0),
sin_f(fsin), saw_f(fsaw), sample_f(fsample),
sin2_f(fsin * 2), dt(1.0/fsample), sin2_len(0.5*fsample/fsin),
pi(3.141592654), c(3.0E8)
{

}

//产生二次谐波信号和相位延迟pi/2的谐波信号
void DSP::generate_sin2_signal(vector<flt> &sin2a, vector<flt> &sin2b)
{
    sin2a.clear();
    sin2b.clear();
    for(unsigned int i = 0; i != sin2_len; ++i)
    {
        sin2a.push_back(sin(4*pi*sin2_f*(i+1)*dt));
        sin2b.push_back(sin((4*sin2_f*(i+1)*dt + 0.5)*pi));
    }
}

//读取滤波器系数
int DSP::read_fir(const string &filename, vector<flt> &data)
{
    data.clear();
    ifstream fir(filename.c_str());
	if (fir.is_open())
	{
    	string s;
    	flt converted = 0;
    	while(fir >> s)
    	{
        	istringstream(s) >> converted;
        	data.push_back(converted);
    	}
		fir.close();
		return 1;
	}
	else
	{
		//std::cout << "Unable to open FIR file." << std::endl;
		return 0;
	}
}

//从文件读取数据，存入vector
int DSP::read_data(const string &filename, vector<int> &data)
{
    data.clear();
    ifstream input(filename.c_str());
	if (input.is_open())
	{
	    string s, s_even, s_odd;
    	unsigned int i = 0;
    	int converted = 0;
    	while(input >> s)
    	{
        	if (i%2 == 0)
            	s_even = s;
        	else
        	{
            	s_odd = s;
            	istringstream(s_even + s_odd) >> std::hex >> converted;
            	data.push_back(converted);
        	}
        	++i;
    	}
    	input.close();
		return 1;
	}
	else
	{
		//std::cout << "Unable to open input file." << std::endl;
		return 0;
	}
}

//从vector读取数据，写入到文件
int DSP::save_results() const
{
    ofstream output(file_output.c_str());
	if (output.is_open())
	{
    	for(vector<flt>::size_type i=0; i!=result.size(); ++i)
    	{
        	output << xaxis[i] << " " <<result[i] << endl;
    	}
    	output.close();
		return 1;
	}
	else
	{
		//std::cout << "Unable to open output file." << std::endl;
		return 0;
	}
}

//DSP核心
int DSP::process()
{
    clock_t start, stop;
    vector<flt> sin2a_signal, sin2b_signal; //å®šä¹‰äºŒæ¬¡è°æ³¢å‚è€ƒä¿¡å·
    vector<flt> fir_coef;                   //FIRæ»¤æ³¢å™¨ç³»æ•°
    vector<flt> laser_signal;                   //laser_signal
    generate_sin2_signal(sin2a_signal, sin2b_signal);
    if (read_fir(file_fir, fir_coef) == 0)                   //ä»Žæ–‡ä»¶è¯»å–FIRæ»¤æ³¢å™¨ç³»æ•°
        return 0;
    //if (read_data(file_input, laser_signal) == 0)            //ä»Žæ–‡ä»¶è¯»å–å…‰ç”µä¿¡å·
    laser_signal = raw;
    laser_len = laser_signal.size();
    fir_len = fir_coef.size();
    vector<flt>::size_type i;
    vector<flt>::size_type j;
    start = clock();                                    //ä¿¡å·å¤„ç†å¼€å§‹
    //è®¡ç®—å¹¶å‡åŽ»å…‰ç”µä¿¡å·çš„å¹³å‡å€¼
    int laser_sum = accumulate(laser_signal.begin(), laser_signal.end(), 0);
    int laser_mean = laser_sum / laser_len;
    for(vector<int>::size_type i = 0; i != laser_len; ++i)
    {
        laser_signal[i] -= laser_mean;
    }

    //å…‰ç”µä¿¡å·ä¸Žè°æ³¢ä¿¡å·ç›¸ä¹˜ï¼Œäº§ç”Ÿä¸¤è·¯æ•°å­—ä¿¡å·
    vector<flt> digit1_signal, digit2_signal;
    for(i = 0; i != laser_len; ++i)
    {
        digit1_signal.push_back(laser_signal[i] * sin2a_signal[i%sin2_len]);
        digit2_signal.push_back(laser_signal[i] * sin2b_signal[i%sin2_len]);
    }
    //å¯¹ä¸¤è·¯æ•°å­—ä¿¡å·è¿›è¡ŒFIRæ»¤æ³¢
    vector<flt> dsp1_signal, dsp2_signal;
    for(i = 0; i != fir_len; ++i)          //è¾“å‡ºèµ·å§‹ä½ç½®è¡¥é›¶
    {
        dsp1_signal.push_back(0.0);
        dsp2_signal.push_back(0.0);
    }
    flt sum1, sum2;
    for(i = fir_len; i != laser_len; ++i)  //å·ç§¯è¿ç®—
    {
        sum1 = 0.0;
        sum2 = 0.0;
        for(j = 0; j != fir_len; ++j)
        {
            sum1 += digit1_signal[i-j] * fir_coef[j];
            sum2 += digit2_signal[i-j] * fir_coef[j];
        }
        dsp1_signal.push_back(sum1);
        dsp2_signal.push_back(sum2);
    }

    //æ»¤æ³¢åŽçš„ä¸¤è·¯ä¿¡å·è‡ªå¹³æ–¹->æ±‚å’Œ->å¼€æ ¹å·
    result.clear();
    xaxis.clear();
    for(i = 0; i != laser_len; ++i)
    {
        xaxis.push_back(dt*i);
        result.push_back(sqrt(dsp1_signal[i] * dsp1_signal[i] + dsp2_signal[i] * dsp2_signal[i]));
    }

    stop = clock();                                     //ä¿¡å·å¤„ç†ç»“æŸ
    elapsed = (stop - start + 0.0)/CLOCKS_PER_SEC;                       //è®¡ç®—å¤„ç†æ—¶é—´
    return 1;

}

int DSP::peak_search()
{
	vector<flt>::size_type n = 8;
    peaks[0].clear();
    peaks[1].clear();
	for(vector<flt>::size_type i = n; i != (laser_len - n); ++i)
	{
		if ((result[i] > result[i+1]) && (result[i] > result[i-1])  && (result[i] > result[i+n]) && (result[i] > result[i-n]) && (result[i] > 0.6*get_maximum()))
		{
			peaks[0].push_back(xaxis[i]);
			peaks[1].push_back(result[i]);
		}
	}
	return 1;
}

int DSP::set_freq(flt fsin, flt fsaw, flt fsample)
{
    if ((fsaw > 0) && (fsin > fsaw) && (fsample >= 4 * fsin))
    {
        sin_f = fsin;
        saw_f = fsaw;
        sample_f = fsample;
        dt = 1.0 / sample_f;
        sin2_f = sin_f * 2.0;
        sin2_len = sample_f / sin2_f;
        return 1;
    }
    else
        return 0;
}

DSP::flt DSP::get_sin_f()
{
	return sin_f;
}

DSP::flt DSP::get_saw_f()
{
	return saw_f;
}

DSP::flt DSP::get_sample_f()
{
	return sample_f;
}

DSP::flt DSP::get_maximum()
{
	return *std::max_element(result.begin(), result.end());
}
//输出结果
ostream& operator << (ostream& output, DSP* dsp)
{
	output << "*************************REPORT*************************" << endl;
	output << "*  Frequency of sine wave: " << dsp->sin_f << " Hz." << endl;
	output << "*  Frequency of sawtooth wave: " << dsp->saw_f << " Hz." << endl;
	output << "*  Sampling frequency: " << dsp->sample_f << " Hz." << endl;
	output << "*  Length of signal: " << dsp->laser_len << "." << endl;
	output << "*  Length of FIR filter: " << dsp->fir_len << "." << endl;
	output << "*  Maximum: " << dsp->get_maximum() << endl;
	output << "*  Time consumed by digital signal processing: " << dsp->elapsed << " s." << endl;
	output << "********************************************************" << endl;
	return output;
}


/*
int DSP::process(vector<DSP::flt> &yVect)
{
    vector<flt> fir_coef;                   //FIR滤波器系数
    vector<flt> laser_signal;                   //laser_signal

    vector<flt> lujy_coef;
    unsigned int lujy_coef_length=0;

    flt sum1, sum2;

    vector<flt>::size_type i;
    vector<flt>::size_type j;

    vector<flt> dsp1_signal;

    if (read_fir("low.fcf", fir_coef) == 0)                   //从文件读取FIR滤波器系数
        return -1;

    if (read_fir("high.fcf", lujy_coef) == 0)                   //从文件读取FIR滤波器系数
        return -1;

    lujy_coef_length=lujy_coef.size();


    laser_signal = yVect;
    laser_len = laser_signal.size();
    fir_len = fir_coef.size();



    //对两路数字信号进行FIR滤波
    for(i = 0; i != fir_len; ++i)          //输出起始位置补零
    {
        dsp1_signal.push_back(0);
    }

 //   qDebug()<<"fcf load success 1"<<fir_len<<laser_len<<lujy_coef[1]<<yVect.size();

    for(i = fir_len; i != laser_len; ++i)  //卷积运算
    {
        sum1 = 0.0;
        for(j = 0; j != fir_len; ++j)
        {
            sum1 += laser_signal[i-j] * fir_coef[j];
        }
        dsp1_signal.push_back(sum1);
    }

    for(i = 0; i != fir_len; ++i)          //输出起始位置补零
    {
        dsp1_signal[i]=dsp1_signal[fir_len];
    }

    unsigned int tempSize=laser_len-fir_len;

    yVect.clear();

//    yVect=dsp1_signal;


    for(i = 0; i != lujy_coef_length; ++i)          //输出起始位置补零
    {
        yVect.push_back(0.0);
    }


    for(i = lujy_coef_length; i != tempSize; ++i)  //卷积运算
    {
        sum2 = 0.0;
        for(j = 0; j != lujy_coef_length; ++j)
        {
            sum2 += dsp1_signal[i-j] * lujy_coef[j];
        }

        yVect.push_back(sum2);
    }

    for(i = 0; i != lujy_coef_length; ++i)          //输出起始位置补零
    {
        yVect[i]=yVect[lujy_coef_length];
    }

    double  max=*max_element(yVect.begin(),yVect.end());


    for(i = 0; i != laser_len; ++i)  //卷积运算
    {
        yVect[i]=max-yVect[i];
    }

//    qDebug()<<"fcf load success2"<<lujy_coef_length<<tempSize<<max;

    return 1;
}
*/



int DSP::process(vector<DSP::flt> &yVect)
{
   
   static float fStandard[70000]={0.0};
  //high pass 1-10hz     0.99859647680112928
   double a[3][6]={1,-1.2901220839446326,0.29012444461671605 , 1 , -1.9988072051152446  , 0.99880853853564711,
                   1, -1.2925412329610346 , 0.29254189156906374 , 1 , -1.9566445635623326  , 0.95700298965928488,
                   1 , -1.9558194992557136, 0.95616393987697879 , 1 , -0.57790127371795708 , 0.085306766648729632};
	//lowpass 500-1000hz		 0.00022253075960123166							 
	 double	b[3][6]={1 , -1.9818034327755296 ,  0.99930328159718584 ,  1 , -1.93519604670219 ,   0.93834832868350182,1,  -1.9120869957867666  , 0.99770074905746386 ,  1 , -1.6511437837010579 , 0.66421276687206598,
                   1 , -0.42721403087975007,  0.042992902313211692 , 1 , -1.9745925367399526 , 0.97956364322303768};						 

		
	 double x1=0.0;
   double x2=0.0;
   double x3=0.0;
   double f1=0.0;
   double f2=0.0;
   double f3=0.0;
   double m01 =0.0;  //中间变量
   double m02 =0.0;
   double m11 =0.0;
   double m12 =0.0;
   double m21 =0.0;
   double m22 =0.0;
int i;	

    for(i = 0; i != 20000; ++i)          //输出起始位置补数组的第一个值
    {
        fStandard[i]=(float)yVect[i];
    }
									 
		for(i = 0; i != 50000; ++i)          
    {
        fStandard[20000+i]=(float)yVect[i];
    }							 

		for(i = 0; i != 70000; ++i) 
		{
        x1=fStandard[i]-a[0][4]*m01-a[0][5]*m02;
        f1=x1*a[0][0]+a[0][1]*m01+a[0][2]*m02;
        m02=m01;
        m01=x1;
        f1=0.99859647680112928  *f1;

        x2=f1-a[1][4]*m11-a[1][5]*m12;
        f2=x2*a[1][0]+a[1][1]*m11+a[1][2]*m12;
        m12=m11;
        m11=x2;

        x3=f2-a[2][4]*m21-a[2][5]*m22;
        f3=x3*a[2][0]+a[2][1]*m21+a[2][2]*m22;
        m22=m21;
        m21=x3;
	
			fStandard[i] = (float)f3;

    }

	  x1=0.0;
    x2=0.0;
    x3=0.0;
    f1=0.0;
    f2=0.0;
    f3=0.0;
    m01 =0.0;  //中间变量
    m02 =0.0;
    m11 =0.0;
    m12 =0.0;
    m21 =0.0;
    m22 =0.0;
		
		for(i = 0; i != 70000; ++i) 
		{
        x1=fStandard[i]-b[0][4]*m01-b[0][5]*m02;
        f1=x1*b[0][0]+b[0][1]*m01+b[0][2]*m02;
        m02=m01;
        m01=x1;
        f1=0.00022253075960123166  *f1;

        x2=f1-b[1][4]*m11-b[1][5]*m12;
        f2=x2*b[1][0]+b[1][1]*m11+b[1][2]*m12;
        m12=m11;
        m11=x2;

        x3=f2-b[2][4]*m21-b[2][5]*m22;
        f3=x3*b[2][0]+b[2][1]*m21+b[2][2]*m22;
        m22=m21;
        m21=x3;
		

       if (i>=20000)		
		    {yVect[i-20000] = (float)f3;}
    }
			
    double  max=*max_element(yVect.begin(),yVect.end());


    for(i = 0; i != 50000; ++i)  //卷积运算
    {
        yVect[i]=max-yVect[i];
    }
//    qDebug()<<"fcf load success2"<<lujy_coef_length<<tempSize<<max;

    return 1;
}








int DSP::processX(vector<DSP::flt> &yVect)
{
//    qDebug()<<"size"<<yVect.size();
    vector<flt> fir_coef;                   //FIR滤波器系数
    vector<flt> laser_signal;                   //laser_signal

    flt sum1;

    vector<flt>::size_type i;
    vector<flt>::size_type j;


    if (read_fir("etalons.fcf", fir_coef) == 0)                   //从文件读取FIR滤波器系数
        return -1;


//    qDebug()<<"help";

    laser_signal = yVect;
    laser_len = laser_signal.size();
    fir_len = fir_coef.size();

    yVect.clear();

    //对两路数字信号进行FIR滤波
    for(i = 0; i != fir_len; ++i)          //输出起始位置补零
    {
        yVect.push_back(0);
    }

    qDebug()<<"fcf load success 1"<<fir_len<<laser_len;

    for(i = fir_len; i != laser_len; ++i)  //卷积运算
    {
        sum1 = 0.0;
        for(j = 0; j != fir_len; ++j)
        {
            sum1 += laser_signal[i-j] * fir_coef[j];
        }
        yVect.push_back(sum1);
    }

    for(i = 0; i != fir_len; ++i)          //输出起始位置补零
    {
        yVect[i]=yVect[fir_len];
    }

    return 1;
}
