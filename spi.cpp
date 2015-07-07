#include "bcm2835.h"
#include "data.h"
#include "spi.h"
using namespace std;
int SPI::read_signal(vector<DSP::flt> &signal)
{
    //bcm2835_spi_begin();
    //bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    //bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // The default
    //bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    //bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    
/*
    char data[20000];  Data to send
	unsigned int data_len = 20000;
   	bcm2835_spi_transfern(data, data_len);
*/

    char data[100000]; // Data to send
	unsigned int data_len = 100000;
   	bcm2835_spi_transfern(data, data_len);
	//bcm2835_spi_end();

    signal.clear();
    /*16bit signal
	for (unsigned int i = 1; i < data_len; i = i + 2)
	{
		signal.push_back(((unsigned char)data[i-1] << 8) | ((unsigned char)data[i]));
	}
    */
    //32bit signal
/*
	for (unsigned int i = 3; i < data_len; i = i + 4)
	{
		signal.push_back((((unsigned long)data[i-3] << 24) | ((unsigned long)data[i-2] << 16) | ((unsigned long)data[i-1] << 8) | ((unsigned long)data[i]))/5.0);
	}
*/

	for (unsigned int i = 1; i < data_len; i = i + 2)
	{
		signal.push_back((((unsigned long)data[i-1] << 8) | ((unsigned long)data[i]))/5.0);
	}
    return 1;
}
