#include "modbus.h"
#include "p24FJ256GB106.h"
#define SENSOR_ID 0x11

int GetMsgLengthFromFunc(unsigned char func_code)
{
	// http://iatips.com/modbus_rtu.html
	switch (func_code)
	{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
			return 8;
		default:
			return MODBUS_UNKNOWN_FUNC;
	}
}

int modbusRecvLoop(void)
{
	// initialise byte counter
	int index = 0;
	int expected_msg_length = -1;
	
	// initialise global state to IDLE
	mb_state = SLAVE_IDLE;

	while ((mb_state == SLAVE_IDLE) || ((mb_state == SLAVE_RECEIVING) && (!mb_req_timeout) && (index != expected_msg_length)))
	{
		if (rdyByte())
		{
			// Enable counting timeout
			start_mb_timeout_timer();
			// Set state to receiving
			mb_state = SLAVE_RECEIVING;
			status_leds_frame_on(); // FRAME LED (Yellow)
			
			if ((index == 1)  && (rx_buffer[0] != SENSOR_ID))
			{
				return MODBUS_NOT_ADDR;
			}
			
			// Calc msg length from func code
			if (index == 2) {
				expected_msg_length = GetMsgLengthFromFunc(rx_buffer[1]);
			}
			rx_buffer[index++] = rxByte();
		}
	}
	status_leds_frame_off();
	close_mb_timeout_timer();
	return index;
}


// process received data buffer
pduErrorType checkPDU(unsigned int pduLength, pduType* msg)
{
	unsigned int crc16 = 0x0000;
	unsigned int x = 0;
	mb_state = SLAVE_CHECKING;

	if (GetMsgLengthFromFunc(rx_buffer[1]) != pduLength) return FUNC_FAIL;

	crc16 = calculate_crc16(rx_buffer, pduLength - 2);
	
	if ((rx_buffer[pduLength-2] != (unsigned char)(crc16 & 0x00ff)) || (rx_buffer[pduLength - 1] != (unsigned char)((crc16 & 0xff00) >> 8)))
		return CRC_FAIL;
	
	msg->address = rx_buffer[0];
	msg->function = rx_buffer[1];
	for (x=0;x<pduLength-4;x++)
	{
	// smart thing to do: make msg->data ptr, and set to address of &rx_buffer[2]
	msg->data[x] = rx_buffer[x+2];
	}
	msg->data_length = pduLength-4; 

	return MSG_OK;
}
