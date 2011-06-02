#define MAX_DATA_LENGTH 32

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
  NOT_ADDR,
  FUNC_FAIL,
  INTERNAL_ERR
} pduErrorType;

extern slaveStateType state; // enum type for state
extern unsigned char rx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)
extern unsigned char tx_buffer[MAX_DATA_LENGTH+4]; // Data size + Addr (1b) + Func (1b) + CRC(2b)

unsigned int modbusRecvLoop(void);
void checkPDU(unsigned int pduLength, pduType* msg);

