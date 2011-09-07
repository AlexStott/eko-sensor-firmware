// Modbus New
#include "include/mb_crc16.h"
#include "include/modbus2.h"


#define MB_DATABUF_REGION_START 	0x3000
#define MB_DATABUF_REGION_LENGTH	64
#define MB_MAX_READ_REG_COUNT		32

#define MB_CONFBUF_REGION_START 	0x8000
#define MB_CONFBUF_REGION_LENGTH 	32

extern void error_led_on( void );
extern unsigned char cfg_eeprom_dirty;

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
char process_pdu( unsigned char* src, unsigned char* dest, unsigned int* databuf, unsigned char* confbuf )
{
	unsigned char idx = 0;
	

	unsigned char func_code;
	
	unsigned int  start_reg;
	unsigned int  reg_count;
	
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
			start_reg = ((unsigned int)( src[2] << 8 ) + ((unsigned int)( src[3])));
			reg_count = ((unsigned int)( src[4] << 8 ) + ((unsigned int)( src[5])));
			return process_pdu_fn3(start_reg, reg_count, confbuf, src, dest);
			break;
		case 0x04:
			start_reg = ((unsigned int)( src[2] << 8 ) + ((unsigned int)( src[3])));
			reg_count = ((unsigned int)( src[4] << 8 ) + ((unsigned int)( src[5])));
			return process_pdu_fn4(start_reg, reg_count, databuf, src, dest);
			break;
		case 0x06:
			start_reg = ((unsigned int)( src[2] << 8 ) + ((unsigned int)( src[3])));
			reg_count = ((unsigned int)( src[4] << 8 ) + ((unsigned int)( src[5])));
			// start_reg => target_reg
			// reg_count => value
			return process_pdu_fn6(start_reg, reg_count, confbuf, src, dest);
			break;
		default:
			error_led_on();
			return response_exception(src, dest, SLAVE_DEVICE_FAILURE);
			break;
	}
	return response_exception(src, dest, ILLEGAL_FUNCTION);
}


char process_pdu_fn4( unsigned int start_reg, unsigned int reg_count, unsigned int* databuf, unsigned char* src, unsigned char* dest )
{
	unsigned int start_idx;
	unsigned int crc16_result;
	int i;
	// check bounds
	if ((start_reg < MB_DATABUF_REGION_START) 
		|| ((start_reg + reg_count - 1) > (MB_DATABUF_REGION_START + MB_DATABUF_REGION_LENGTH - 1))
		|| (reg_count == 0))
		return response_exception(src, dest, ILLEGAL_DATA_ADDRESS);
	
	// if registers to read more than size available in txbuffer
	if (reg_count > MB_MAX_READ_REG_COUNT)
		return response_exception(src, dest, SLAVE_DEVICE_FAILURE);
		
	start_idx = start_reg - MB_DATABUF_REGION_START; // remove offset
	
	dest[2] = (unsigned char)(reg_count * 2); // bytes to follow
	
	for (i = 0; i < reg_count; i++)
	{
		dest[3 + 2*i] = (unsigned char)(databuf[start_idx + i] >> 8);
		dest[4 + 2*i] = (unsigned char)(0x00FF & databuf[start_idx + i]);
	}
	
	crc16_result = calculate_crc16(dest, 3 + reg_count*2);
	dest[3 + reg_count*2] = (unsigned char)(crc16_result & 0x00FF);
	dest[3 + reg_count*2 + 1] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	return (3 + reg_count*2 + 2);
}




char process_pdu_fn3( unsigned int start_reg, unsigned int reg_count, unsigned char* confbuf, unsigned char* src, unsigned char* dest )
{
	unsigned int start_idx;
	unsigned int crc16_result;
	int i;
	// check bounds
	if ((start_reg < MB_CONFBUF_REGION_START) 
		|| ((start_reg + reg_count - 1) > (MB_CONFBUF_REGION_START + MB_CONFBUF_REGION_LENGTH - 1))
		|| (reg_count == 0))
		return response_exception(src, dest, ILLEGAL_DATA_ADDRESS);
	
	// if registers to read more than size available in txbuffer
	if (reg_count > MB_MAX_READ_REG_COUNT)
		return response_exception(src, dest, SLAVE_DEVICE_FAILURE);
		
	start_idx = start_reg - MB_CONFBUF_REGION_START; // remove offset
	
	dest[2] = (unsigned char)(reg_count * 2); // bytes to follow
	
	for (i = 0; i < reg_count; i++)
	{
		dest[3 + 2*i] = confbuf[start_idx + 2*i];
		dest[4 + 2*i] = confbuf[start_idx + 2*i + 1];
	}
	
	crc16_result = calculate_crc16(dest, 3 + reg_count*2);
	dest[3 + reg_count*2] = (unsigned char)(crc16_result & 0x00FF);
	dest[3 + reg_count*2 + 1] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	return (3 + reg_count*2 + 2);
}

char process_pdu_fn6( unsigned int target_reg, unsigned int target_value, unsigned char* confbuf, unsigned char* src, unsigned char* dest )
{
	unsigned int target_idx;

	
	// check bounds
	if ((target_reg < MB_CONFBUF_REGION_START)
		|| (target_reg > MB_CONFBUF_REGION_START + MB_CONFBUF_REGION_LENGTH - 1))
		return response_exception(src, dest, ILLEGAL_DATA_ADDRESS);
	
	target_idx = target_reg - MB_CONFBUF_REGION_START;
	
	dest[2] = src[2];
	dest[3] = src[3];
	dest[4] = src[4];
	dest[5] = src[5];
	dest[6] = src[6];
	dest[7] = src[7];
	
	if ((target_idx >= 8) || ((confbuf[16] == 0xE0) && (confbuf[17] == 0x10)))
	{
		// we overwrite the entire register, not just one byte!
		confbuf[2*target_idx] = (unsigned char)((target_value & 0xFF00) >> 8);
		confbuf[2*target_idx + 1] = (unsigned char)(target_value & 0x00FF);
		if (target_idx < 8)
			cfg_eeprom_dirty = 1;
	} else
	{
		return response_exception(src, dest, SLAVE_DEVICE_FAILURE);
	}
	
	
	return 8;
}
