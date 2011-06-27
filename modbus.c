#include "global.h"
#include "SensorProfiles.h"
#include "modbus.h"
#include "adc.h"
#include "p24FJ256GB106.h"

#define SENSOR_ID 0xFA

extern	unsigned int	data1[10];
	
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

unsigned int check_function_supported(unsigned char mb_func_code)
{
	switch (mb_func_code)
	{
		//case 0x00: check if broadcast!
		#ifdef SUPPORT_MB01
		case 0x01:
		#endif
		#ifdef SUPPORT_MB02
		case 0x02:
		#endif
		#ifdef SUPPORT_MB03
		case 0x03:
		#endif
		#ifdef SUPPORT_MB04
		case 0x04:
		#endif
		#ifdef SUPPORT_MB05
		case 0x05:
		#endif
		#ifdef SUPPORT_MB06
		case 0x06:
		#endif
		#ifdef SUPPORT_MB07
		case 0x07:
		#endif
		#ifdef SUPPORT_MB08
		case 0x08;
		#endif
			return 1;
		default:
			return 0;
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
			
			if ((index == 1)  && (rx_buffer[0] != MODBUS_ID))
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


pduErrorType process_mb_function04(unsigned int start_reg, unsigned int count)
{
	unsigned int start_index, i;
	start_index = start_reg - MB04_OFFSET;
	for (i = 0; i < count; i++)
	{
		mb_resp_pdu.data[2*i] = data1[i] >> 8;
		mb_resp_pdu.data[2*i+1] = (unsigned char)(data1[i] & 0x00ff);
	}
	mb_resp_pdu.data_length = 2*count;
	return MSG_OK;
}

pduErrorType process_req_pdu(void)
{
	mb_state = SLAVE_PROCESSING;
	unsigned int data_address_start, data_address_count;
	
	switch (mb_req_pdu.function)
	{
		case 0x04:
			data_address_start = ((unsigned short)( mb_req_pdu.data[0]) << 8 ) + ((unsigned short)( mb_req_pdu.data[1]));
			data_address_count  = ((unsigned short)( mb_req_pdu.data[2]) << 8 ) + ((unsigned short)( mb_req_pdu.data[3]));
			if ((data_address_start >= MB04_OFFSET) && (data_address_start+data_address_count <= MB04_END))
			{
				return process_mb_function04(data_address_start, data_address_count);
			}
		case 0x03:
			// Implement
		default:
			return PARAM_ERROR;
	}
}


unsigned char format_resp_pdu(pduErrorType status)
{
	unsigned int i, crc16;
	tx_buffer[0] = mb_req_pdu.address;
	tx_buffer[1] = mb_req_pdu.function;
	if (status == MSG_OK)
	{
		tx_buffer[2] = mb_resp_pdu.data_length;
		for (i = 0; i < mb_resp_pdu.data_length; i++)
		{
			tx_buffer[3+i] = mb_resp_pdu.data[i];
		}
		crc16 = calculate_crc16(tx_buffer, 3+mb_resp_pdu.data_length);
		tx_buffer[3+mb_resp_pdu.data_length] = (unsigned char)(crc16 & 0x00ff);
		tx_buffer[3+mb_resp_pdu.data_length+1]	   = (unsigned char)(crc16 >> 8);
		tx_buffer[3+mb_resp_pdu.data_length+2] = '\0';
		return 3+mb_resp_pdu.data_length+2;
	}
	else
	{
		tx_buffer[1] = tx_buffer[1] | 0x80;
		tx_buffer[2] = (unsigned char)status;
		crc16 = calculate_crc16(tx_buffer, 3);
		tx_buffer[3] = (unsigned char)(crc16 & 0x00ff);
		tx_buffer[4]	   = (unsigned char)(crc16 >> 8);
		return 5;
	}
}


// process received data buffer
pduErrorType check_req_pdu(unsigned int pduLength)
{
	unsigned int crc16 = 0x0000;
	unsigned int x = 0;
	mb_state = SLAVE_CHECKING;

	if (GetMsgLengthFromFunc(rx_buffer[1]) != pduLength) return MSG_LEN_ERROR;

	crc16 = calculate_crc16(rx_buffer, pduLength - 2);
	
	if ((rx_buffer[pduLength-2] != (unsigned char)(crc16 & 0x00ff)) || (rx_buffer[pduLength - 1] != (unsigned char)((crc16 & 0xff00) >> 8)))
		return CRC_FAIL;
	
	
	mb_req_pdu.address = rx_buffer[0];
	mb_req_pdu.function = rx_buffer[1];
	for (x=0;x<pduLength-4;x++)
	{
	mb_req_pdu.data[x] = rx_buffer[x+2];
	}
	mb_req_pdu.data_length = pduLength-4; 
	
	if (!check_function_supported(mb_req_pdu.function)) return FUNC_UNSUPPORTED;
	
	return MSG_OK;
}
