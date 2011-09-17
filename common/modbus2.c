// Modbus New
#include "include/mb_crc16.h"
#include "include/modbus2.h"
#include "board/p24_board.h"

#define MB_DATABUF_REGION_START 	0x3000
#define MB_DATABUF_REGION_LENGTH	256
#define MB_MAX_READ_REG_COUNT		32

#define MB_CONFBUF_REGION_START 	0x8000
// 64 bytes
#define MB_CONFBUF_REGION_LENGTH 	32


extern void error_led_on( void );
extern unsigned char cfg_eeprom_dirty;

extern unsigned char rxbuf[];
extern unsigned char txbuf[];

extern unsigned int databuf[];
extern unsigned char confbuf[];


char validate_pdu( unsigned char daddr, unsigned char msg_len)
{
	unsigned int crc16;
	// unsigned int temp;
	// check address
	if (rxbuf[0] != daddr) return 0;

	// check CRC
	crc16 = (((rxbuf[msg_len-1] << 8) & 0xFF00) + (0x00FF & rxbuf[msg_len-2]));
	// temp = calculate_crc16(rxbuf, msg_len-2);
	if (crc16 != calculate_crc16(rxbuf, msg_len-2)) return -1;
	
	confbuf[SYS_MB_MSG_COUNT] += 1;
	return 1;
}

char response_exception( unsigned char errcode)
{
	unsigned int crc16_result;
	unsigned char idx = 1;
	error_led_on();
	txbuf[idx] = txbuf[idx] | 0x80;
	txbuf[++idx] = errcode;
	crc16_result = calculate_crc16(txbuf, 3);
	txbuf[++idx] = (unsigned char)(crc16_result & 0x00FF);
	txbuf[++idx] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	confbuf[SYS_MB_ERR_COUNT] += 1;
	return idx+1;
}

// returns number of bytes to transmit
char process_pdu( void )
{
	unsigned char idx = 0;
	

	unsigned char func_code;
	
	unsigned int  start_reg;
	unsigned int  reg_count;
	
	txbuf[idx] = rxbuf[idx]; // copy address
	idx++; // next position
	
	func_code = rxbuf[idx];
	txbuf[idx] = func_code;
	
	if (!MB_SUPPORTED_FUNC(func_code))
	{
		// illegal function
		return response_exception(ILLEGAL_FUNCTION);
	}
	
	// dispatch function code
	switch (func_code)
	{
		case 0x03:
			start_reg = ((unsigned int)( rxbuf[2] << 8 ) + ((unsigned int)( rxbuf[3])));
			reg_count = ((unsigned int)( rxbuf[4] << 8 ) + ((unsigned int)( rxbuf[5])));
			return process_pdu_fn3(start_reg, reg_count);
			break;
		case 0x04:
			start_reg = ((unsigned int)( rxbuf[2] << 8 ) + ((unsigned int)( rxbuf[3])));
			reg_count = ((unsigned int)( rxbuf[4] << 8 ) + ((unsigned int)( rxbuf[5])));
			return process_pdu_fn4(start_reg, reg_count);
			break;
		case 0x06:
			start_reg = ((unsigned int)( rxbuf[2] << 8 ) + ((unsigned int)( rxbuf[3])));
			reg_count = ((unsigned int)( rxbuf[4] << 8 ) + ((unsigned int)( rxbuf[5])));
			// start_reg => target_reg
			// reg_count => value
			return process_pdu_fn6(start_reg, reg_count);
			break;
		default:
			return response_exception(SLAVE_DEVICE_FAILURE);
			break;
	}
	return response_exception(ILLEGAL_FUNCTION);
}


char process_pdu_fn4( unsigned int start_reg, unsigned int reg_count)
{
	unsigned int start_idx;
	unsigned int crc16_result;
	int i;
	// check bounds
	if ((start_reg < MB_DATABUF_REGION_START) 
		|| ((start_reg + reg_count - 1) > (MB_DATABUF_REGION_START + MB_DATABUF_REGION_LENGTH - 1))
		|| (reg_count == 0))
		return response_exception(ILLEGAL_DATA_ADDRESS);
	
	// if registers to read more than size available in txbuffer
	if (reg_count > MB_MAX_READ_REG_COUNT)
		return response_exception(SLAVE_DEVICE_FAILURE);
		
	start_idx = start_reg - MB_DATABUF_REGION_START; // remove offset
	
	txbuf[2] = (unsigned char)(reg_count * 2); // bytes to follow
	
	for (i = 0; i < reg_count; i++)
	{
		txbuf[3 + 2*i] = (unsigned char)(databuf[start_idx + i] >> 8);
		txbuf[4 + 2*i] = (unsigned char)(0x00FF & databuf[start_idx + i]);
	}
	
	crc16_result = calculate_crc16(txbuf, 3 + reg_count*2);
	txbuf[3 + reg_count*2] = (unsigned char)(crc16_result & 0x00FF);
	txbuf[3 + reg_count*2 + 1] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	return (3 + reg_count*2 + 2);
}




char process_pdu_fn3( unsigned int start_reg, unsigned int reg_count )
{
	unsigned int start_idx = 0;
	unsigned int crc16_result = 0;
	int i=0;
	// check bounds
	if ((start_reg < MB_CONFBUF_REGION_START) 
		|| ((start_reg + reg_count - 1) > (MB_CONFBUF_REGION_START + MB_CONFBUF_REGION_LENGTH - 1))
		|| (reg_count == 0))
		return response_exception(ILLEGAL_DATA_ADDRESS);
	
	// if registers to read more than size available in txbuffer
	if (reg_count > MB_MAX_READ_REG_COUNT)
		return response_exception( SLAVE_DEVICE_FAILURE);
		
	start_idx = start_reg - MB_CONFBUF_REGION_START; // remove offset
	// idx indexes words, we read from bytes, so remember to use idx*2
	txbuf[2] = (unsigned char)(reg_count * 2); // bytes to follow
	
	for (i = 0; i < reg_count; i++)
	{
		txbuf[3 + 2*i] = confbuf[2*start_idx + 2*i];
		txbuf[4 + 2*i] = confbuf[2*start_idx + 2*i + 1];
	}
	
	crc16_result = calculate_crc16(txbuf, 3 + reg_count*2);
	txbuf[3 + reg_count*2] = (unsigned char)(crc16_result & 0x00FF);
	txbuf[3 + reg_count*2 + 1] = (unsigned char)((crc16_result >> 8) & 0x00FF);
	return (3 + reg_count*2 + 2);
}

char process_pdu_fn6( unsigned int target_reg, unsigned int target_value)
{
	unsigned int target_idx;

	
	// check bounds
	if ((target_reg < MB_CONFBUF_REGION_START)
		|| (target_reg > MB_CONFBUF_REGION_START + MB_CONFBUF_REGION_LENGTH - 1))
		return response_exception(ILLEGAL_DATA_ADDRESS);
	
	target_idx = target_reg - MB_CONFBUF_REGION_START;
	
	txbuf[2] = rxbuf[2];
	txbuf[3] = rxbuf[3];
	txbuf[4] = rxbuf[4];
	txbuf[5] = rxbuf[5];
	txbuf[6] = rxbuf[6];
	txbuf[7] = rxbuf[7];
	
	if (((target_idx >= 8) || ((confbuf[CFG_EE_LOCK_HI] == 0xE0) && (confbuf[CFG_EE_LOCK_LO] == 0x10))) && (target_idx < (DAT_I2C_TEMP_HI >> 1) ))
	{
		// we overwrite the entire register, not just one byte!
		confbuf[2*target_idx] = (unsigned char)((target_value & 0xFF00) >> 8);
		confbuf[2*target_idx + 1] = (unsigned char)(target_value & 0x00FF);
		if (target_idx < 8)
			cfg_eeprom_dirty = 1;
	} else
	{
		return response_exception(SLAVE_DEVICE_FAILURE);
	}
	
	
	return 8;
}
