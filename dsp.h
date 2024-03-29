#ifndef DSP_H_
#define DSP_H_


#include <string>
#include <vector>



using namespace std;

class DSP
{
public:
	typedef double flt;
    DSP(const string input = "raw.txt", const string fir = "fir.fcf", const string output = "results.txt", const flt fsin = 200.0, const flt fsaw = 1.0, const flt fsample = 10000.0);
    vector<flt> result;	//输出信号（二次谐波系数）
    vector<flt> raw;
    vector<flt> xaxis;
	vector<flt> peaks[2];
	string file_input;
	string file_fir;
	string file_output;
	unsigned int laser_len;					//for test only
	unsigned int fir_len;					//for test only
	float elapsed;							//运行时间
	int set_freq(flt, flt, flt);
	flt get_sin_f();
	flt get_saw_f();
	flt get_sample_f();
	int process();
	int peak_search();
	flt get_maximum();
	int save_results() const;
	friend ostream& operator << (ostream&, DSP*);

    //2015-05-25 lujy
    int process(vector<flt> &);
    int processX(vector<flt> &);

private:
	flt sin_f;                          	//正弦波频率
	flt saw_f;                          	//锯齿波频率
	flt sample_f;                     		//采样频率（需要保证采样频率是正弦波频率的20倍以上）
	flt sin2_f;                     		//二次谐波频率
	flt dt;                      			//采样时间间隔
	unsigned int sin2_len;      			//二次谐波信号长度, implicit conversion is used (flt to int)
	const flt pi;                       	//圆周率
	const flt c;                        	//光速，单位：m/s
	void generate_sin2_signal(vector<flt> &, vector<flt> &);
	int read_data(const string &, vector<int> &);
	int read_fir(const string &, vector<flt> &);
};
#endif /* DSP_H_ */
