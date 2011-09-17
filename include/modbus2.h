#ifndef _MODBUS_2_H
#define MODBUS_2_H


#define	ILLEGAL_FUNCTION 			0x01
#define	ILLEGAL_DATA_ADDRESS 	0x02
#define	ILLEGAL_DATA_VALUE 		0x03
#define	SLAVE_DEVICE_FAILURE 	0x04
#define	ACKNOWLEDGE 					0x05
#define	SLAVE_DEVICE_BUSY 		0x06

#define MB_SUPPORTED_FUNC(x)	( (x == 0x03) || (x == 0x04) || ( x == 0x06) )


char response_exception( unsigned char errcode);
char validate_pdu( unsigned char daddr, unsigned char msg_len );

char process_pdu( void );

char process_pdu_fn4( unsigned int start_reg, unsigned int reg_count );

char process_pdu_fn3( unsigned int start_reg, unsigned int reg_count );
char process_pdu_fn6( unsigned int target_reg, unsigned int target_value );



#endif
