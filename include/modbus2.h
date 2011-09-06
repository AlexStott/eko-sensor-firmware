#ifndef _MODBUS_2_H
#define MODBUS_2_H


#define	ILLEGAL_FUNCTION 		0x01
#define	ILLEGAL_DATA_ADDRESS 	0x02
#define	ILLEGAL_DATA_VALUE 		0x03
#define	SLAVE_DEVICE_FAILURE 	0x04
#define	ACKNOWLEDGE 			0x05
#define	SLAVE_DEVICE_BUSY 		0x06

#define MB_SUPPORTED_FUNC(x)	( (x == 0x03) || (x == 0x04) || ( x == 0x06) )


char response_exception( unsigned char* src, unsigned char* dest, unsigned char errcode);
char validate_pdu( unsigned char daddr, unsigned char msg_len, unsigned char* ptr );

char process_pdu( unsigned char* src, unsigned char* dest );



#endif