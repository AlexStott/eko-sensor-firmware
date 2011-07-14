#include "global.h"
#include "SensorProfiles.h"
#include "modbus.h"
#include "adc.h"
#include "p24Fxxxx.h"
#include "eko_i2c_sensors.h"
#define SENSOR_ID 0xFA

unsigned int message_count;
unsigned int error_count;
unsigned int crcfail_count;
	
int GetMsgLengthFromFunc(unsigned char func_code)
{
	// http://iatips.com/modbus_rtu.html
	switch (func_code)
	{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
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
	int val = 0;
	// initialise global state to IDLE
	mb_state = SLAVE_IDLE;
	status_leds_frame_off();
	while ((mb_state == SLAVE_IDLE) || ((mb_state == SLAVE_RECEIVING) && (!mb_req_timeout) && (index != expected_msg_length)))
	{
		if (rdyByte())
		{
			status_leds_frame_on();
			// Enable counting timeout
			start_mb_timeout_timer();
			// Set state to receiving
			mb_state = SLAVE_RECEIVING;
			
			
			//if ((index == 1)  && (rx_buffer[0] != MODBUS_ID))
			//{
			//	val = MODBUS_NOT_ADDR;
				//status_leds_frame_off();
			//}
			//if ((val == MODBUS_NOT_ADDR) && index > 1)
			//{
				//status_leds_frame_on(); // FRAME LED (Yellow)
			//}
			
			// Calc msg length from func code
			if (index == 2) {
				expected_msg_length = GetMsgLengthFromFunc(rx_buffer[1]);
			}
			rx_buffer[index++] = rxByte();
		//	IFS0bits.U1RXIF = 1;
		}
	}
	status_leds_frame_off();
	close_mb_timeout_timer();
	//if (val != MODBUS_NOT_ADDR)
		return index;
	//else
	//	mb_state = SLAVE_IDLE;
	//	return MODBUS_NOT_ADDR;
}


pduErrorType process_mb_function04(unsigned int start_reg, unsigned int count)
{
	unsigned int start_index, i, adc_channel;
	unsigned int temp;
	// Figure out which block it is in (i2c, analog)
	if ((start_reg >= 0x5000) && (start_reg <= 0x53FF))
	{
		// Sequential read from EEPROM
		return INTERNAL_ERR;
	}
	else if ((start_reg >= 0x4000) && (start_reg <= 0x4FFF))
	{
		// Sequential read from ADC buffers	
		// figure out which ADC channel was requested (the n in 0x4n00, trailing two zeros address each word in buffer)
		adc_channel = (start_reg & 0x0F00) >> 8;
		
		// start address
		start_index = (start_reg & 0x00FF);
		
		// check bounds
		if (adc_channel >= 3) return ADDR_ERROR;
		if ((start_index+count) > MAX_DATA_LENGTH) return ADDR_ERROR;
		
		for (i = 0; i < count; i++)
		{
			mb_resp_pdu.data[2*i] = analog_buffer[adc_channel][start_index+i] >> 8;
			mb_resp_pdu.data[2*i+1] = (unsigned char)(analog_buffer[adc_channel][start_index+i] & 0x00ff);
		}
		mb_resp_pdu.data_length = 2*count;
		return MSG_OK;
	}
	#ifdef SENSORKIT_ATMOS_R1
	else if (start_reg == I2C_TSL2561_LUX)
	{
		temp = tsl2561_get_lux();
		mb_resp_pdu.data[0] = (unsigned char)(temp >> 8);
		mb_resp_pdu.data[1] = (unsigned char)(temp & 0x00ff);
		mb_resp_pdu.data_length = 2;
		return MSG_OK;
	} 
	else if (start_reg == I2C_MCP9800_TA)
	{
		temp = mcp9800_get_temp();
		mb_resp_pdu.data[0] = (unsigned char)(temp >> 8);
		mb_resp_pdu.data[1] = (unsigned char)(temp & 0x00ff);
		mb_resp_pdu.data_length = 2;
		return MSG_OK;
	}
	#endif
	else if (start_reg == 0x1000)
	{
		mb_resp_pdu.data[0] = (unsigned char)(TYPE_CODE >> 8);
		mb_resp_pdu.data[1] = (unsigned char)(TYPE_CODE & 0x00ff);
		mb_resp_pdu.data_length = 2;
		return MSG_OK;
	}
	else
	{
		// data address outside bounds
		return ADDR_ERROR;
	}
	
}

pduErrorType process_mb_function06(unsigned int target_reg, unsigned int value)
{
	if((target_reg >= 0x5000) && (target_reg <= 0x53FF))
	{
		// write to EEPROM
		return INTERNAL_ERR;
	}
	else if ((target_reg == 0x6500))
	{
		// Call ADC main. Disregard value for now
		ADCmain();
		mb_resp_pdu.data[0] = mb_req_pdu.data[0];
		mb_resp_pdu.data[1] = mb_req_pdu.data[1];
		mb_resp_pdu.data_length = 2;
		return MSG_OK;
	}

	return ADDR_ERROR;
	
	
}

pduErrorType process_req_pdu(void)
{
	mb_state = SLAVE_PROCESSING;
	unsigned int data_field_1, data_field_2;
	
	switch (mb_req_pdu.function)
	{
		case 0x03:
		case 0x04:
			// start reading at
			data_field_1= ((unsigned short)( mb_req_pdu.data[0]) << 8 ) + ((unsigned short)( mb_req_pdu.data[1]));
			// data count (to read)
			data_field_2  = ((unsigned short)( mb_req_pdu.data[2]) << 8 ) + ((unsigned short)( mb_req_pdu.data[3]));
			if ((data_field_1 >= MB04_OFFSET) && (data_field_1+data_field_2 <= MB04_END))
			{
				return process_mb_function04(data_field_1, data_field_2);
			}
		case 0x06:
			// write to address
			data_field_1= ((unsigned short)( mb_req_pdu.data[0]) << 8 ) + ((unsigned short)( mb_req_pdu.data[1]));
			// value
			data_field_2  = ((unsigned short)( mb_req_pdu.data[2]) << 8 ) + ((unsigned short)( mb_req_pdu.data[3]));
			if ((data_field_1 >= MB06_OFFSET) && (data_field_2 < MB06_END))
			{
				return process_mb_function06(data_field_1, data_field_2);
			}
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
		if (mb_req_pdu.function == 0x06)
		{
			for (i=0;i<8;i++)
			{
				tx_buffer[i] = rx_buffer[i];
			}
			return 8;
		}
		else
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
	}
	else if (status > 0)
	{
		tx_buffer[1] = tx_buffer[1] | 0x80;
		tx_buffer[2] = (unsigned char)status;
		crc16 = calculate_crc16(tx_buffer, 3);
		tx_buffer[3] = (unsigned char)(crc16 & 0x00ff);
		tx_buffer[4]	   = (unsigned char)(crc16 >> 8);
		return 5;
	} else
	{
		// do nothing if its a CRC failure
		return 0;
	}
	return 0;
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
