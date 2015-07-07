#ifndef SPI_H_
#define SPI_H_
#include <vector>
#include "dsp.h"
using namespace std;
class SPI {
public:
	static int read_signal(vector<DSP::flt> &);
};
#endif /* SPI_H_ */
