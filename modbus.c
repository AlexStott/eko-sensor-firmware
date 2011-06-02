#define TARG_PIC24F_SK
#include "HardwareProfiles.h"
#include "modbus.h"
#include <uart.h>
#define TIMER_3C5 2000
#define SENSOR_ID 0x11

unsigned int modbusRecvLoop(void)
{
  // initialise everything
  unsigned int index = 0;
  state = SLAVE_IDLE; 
  while ((state == SLAVE_IDLE) || ((state == SLAVE_RECEIVING) && (TMR1 < TIMER_3C5 )))
  {
    if (DataRdyUART1())
    {
	state = SLAVE_RECEIVING;
	LATFbits.LATF4 = 0;
	LATGbits.LATG6 = 1;
	LATGbits.LATG8 = 0;
    TMR1 = 0;
    rx_buffer[index++] = ReadUART1();
    
      
    }
  }

  return index;
}

// process received data buffer
void checkPDU(unsigned int pduLength, pduType* msg)
{
  int x;
  state = SLAVE_CHECKING;
  if (rx_buffer[0] != SENSOR_ID)
    //return NOT_ADDR;
	{
	LATFbits.LATF4 = 1;
	LATGbits.LATG6 = 1;
	LATGbits.LATG8 = 1;
	while (TMR1 < 20000);
	return; 
	}
  /* Insert CRC checking code here. See PIC24F peripheral library */
  //if (!checkCRC)
   // return CRC_FAIL;
  // fill struct
  msg->address = rx_buffer[0];
  msg->function = rx_buffer[1];
  for (x=0;x<pduLength-4;x++)
  {
    // smart thing to do: make msg->data ptr, and set to address of &rx_buffer[2]
    msg->data[x] = rx_buffer[x+2];
	
  }
  msg->data_length = pduLength-4; 
	LATFbits.LATF4 = 1;
	LATGbits.LATG6 = 0;
	LATGbits.LATG8 = 0;
	while (TMR1 < 20000);  
  //return MSG_OK;
return;
}
