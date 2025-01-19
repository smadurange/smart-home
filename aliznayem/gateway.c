#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>

#include "RFM69.h"
#include "RFM69registers.h"

#include "serial.h"

#define NETWORKID 33
#define NODEID     4

int main(void)
{
    // initialize RFM69
	serial_init();

    rfm69_init(433, NODEID, NETWORKID);
    setHighPower(1);   // if model number rfm69hw
    setPowerLevel(30); // 0-31; 5dBm to 20 dBm 
    encrypt(NULL);     // if set has to be 16 bytes. example: "1234567890123456"

    // initialize 16x2 LCD
  
    while (1) 
    {
	if(receiveDone())
        {
            _delay_ms(10);
            if(ACKRequested())
	    {
		char ack[0];
                sendACK(ack, 0);
	    }
            char stringData[17];
            for(uint8_t i=0;i<16;i++) // max 16 digit can be shown in this case
            {
                stringData[i]=DATA[i];
            }

			stringData[16] = 0;
            serial_write_line(stringData);
        } else {
            serial_write_line("No data");
            _delay_ms(500);
		}
    }
}
