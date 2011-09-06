// Modbus New
#include "include/mb_crc16.h"
#include "include/modbus2.h"

extern void error_led_on( void );

int adc_samples[5][32];


char validate_pdu( unsigned char daddr, unsigned char msg_len, unsigned char* ptr )
{
	unsigned int crc16;
	// unsigned int temp;
	// check address
	if (ptr[0] != daddr) return 0;

	// check CRC
	crc16 = (((ptr[msg_len-1] << 8) & 0xFF00) + (0x00FF & ptr[msg_len-2]));
	// temp = calculate_crc16(ptr, msg_len-2);
	if (crc16 != calculate_crc16(ptr, msg_len-2)) return -1;

	return 1;
}

char response_exception( unsigned char* src, unsigned char* dest, unsigned char errcode)
{
	unsigned int crc16_result;
	unsigned char idx = 1;
	dest[idx] = dest[idx] | 0x80;
	dest[++idx] = errcode;
	crc16_result = calculate_crc16(dest, 3);
	dest[++idx] = (unsigned char)(crc16_result & 0x00FF);
	dest[++idx] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	return idx+1;
}

// returns number of bytes to transmit
char process_pdu( unsigned char* src, unsigned char* dest )
{
	unsigned char idx = 0;
	unsigned char func_code;
	
	dest[idx] = src[idx]; // copy address
	idx++; // next position
	func_code = src[idx];
	dest[idx] = func_code;
	if (!MB_SUPPORTED_FUNC(func_code))
	{
		// illegal function
		error_led_on();
		return response_exception(src, dest, ILLEGAL_FUNCTION);
	}
	
	// dispatch function code
	switch (func_code)
	{
		case 0x03:
			
		case 0x04:
			break;
		case 0x06:
			break;
		default:
			error_led_on();
			return response_exception(src, dest, SLAVE_DEVICE_FAILURE);
			break;
	}
	return response_exception(src, dest, ILLEGAL_FUNCTION);
}
