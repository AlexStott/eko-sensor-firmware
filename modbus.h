#define MAX_DATA_LENGTH 32

struct pduType
{
  unsigned char address;  // device address (8 bits)
  unsigned char function; // function code (8 bits)
  unsigned char data[MAX_DATA_LENGTH]; // Data buffer length
  unsigned char data_length; // Length of data in buffer
} ;

enum slaveStateType {
  SLAVE_IDLE = 0, // chip is idle, no data in received buffer
  SLAVE_RECEIVING, // receiving data, buffer partially full
  SLAVE_CHECKING, // decoding received message, no new data
  SLAVE_PROCESSING, // processing required action
  SLAVE_FORMATTING, // preparing reply with data
  SLAVE_TRANSMITTING, //putting output data into UART1
  SLAVE_REQ_ERR = 10, // preparing improper request error reply
  SLAVE_PROC_ERR // preparing processing error reply
};


enum pduErrorType {
  MSG_OK = 0,
  CRC_FAIL,
  NOT_ADDR,
  FUNC_FAIL,
  INTERNAL_ERR
} ;
