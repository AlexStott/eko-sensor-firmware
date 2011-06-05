#define MAX_DATA_LENGTH 32

#define MODBUS_NOT_ADDR				-1
#define MODBUS_UNKNOWN_FUNC		-10

typedef struct 
{
  unsigned char address;  // device address (8 bits)
  unsigned char function; // function code (8 bits)
  unsigned char data[MAX_DATA_LENGTH]; // Data buffer length
  unsigned char data_length; // Length of data in buffer
} pduType;

typedef enum {
  SLAVE_IDLE = 0, // chip is idle, no data in received buffer
  SLAVE_RECEIVING, // receiving data, buffer partially full
  SLAVE_CHECKING, // decoding received message, no new data
  SLAVE_PROCESSING, // processing required action
  SLAVE_FORMATTING, // preparing reply with data
  SLAVE_TRANSMITTING, //putting output data into UART1
  SLAVE_REQ_ERR = 10, // preparing improper request error reply
  SLAVE_PROC_ERR // preparing processing error reply
} slaveStateType;


typedef enum  {
  MSG_OK = 0,
  CRC_FAIL,
  FUNC_FAIL,
  INTERNAL_ERR
} pduErrorType;

extern slaveStateType mb_state; // enum type for state

extern unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
extern unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)

extern volatile unsigned char mb_req_timeout;

extern void txByte(unsigned char byte);
extern unsigned char rxByte();
extern unsigned char rdyByte();

extern void status_leds_frame_off();
extern void status_leds_frame_on();

extern unsigned int calculate_crc16(unsigned char *buffer, unsigned int length);
extern void start_mb_timeout_timer();
extern void close_mb_timeout_timer();
extern void DebugLED1();
extern void DebugLED2();
extern void DebugLED3();

int modbusRecvLoop(void);
pduErrorType checkPDU(unsigned int pduLength, pduType* msg);

