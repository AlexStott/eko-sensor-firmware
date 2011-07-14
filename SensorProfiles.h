/* Defenitions for Sensor Boards / Customisations */

/* Register Offsets
 * Analog Inputs:
 *		Analog Channels:	0x3000 (base)
		AN0					0x3000 (offset 0)
		AN1					0x3100
		AN5					0x3500
		
 *	Digital Inputs
 *		i2c Registers			0x5000
 *		Frequency				0x400
 *		CTMU					0x500
 */
 
/* MODBUS addresses for sensor data buffers */
#define AN_BASE			   		 0x4000
#define AN0_OFFSET			0x0000
#define AN1_OFFSET			0x0100
#define AN5_OFFSET			0x0500

#define I2C_EEPROM_BASE		0x5000
#define	I2C_EEPROM_BLK1		0x5000
#define I2C_EEPROM_BLK2		0x5100
#define I2C_EEPROM_BLK3		0x5200
#define I2C_EEPROM_BLK4		0x5300


#define I2C_TSL2561_LUX		0x5400

#define I2C_MCP9800_TA		0x5500

#define SUPPORT_MB04 // support read from read-only register
#define SUPPORT_MB03 // support read from writable register
#define SUPPORT_MB06 // Support write to single register

#ifdef SENSORKIT_EKO_TEST
	#define TYPE_CODE 0x0000
	#define MODBUS_ID 0x01 // fixme
#endif

#ifdef SENSORKIT_DCPWR_R1
	#define TYPE_CODE 0x00DC
	#define MODBUS_ID 0x02
#endif

#ifdef SENSORKIT_ACPWR_R1
	#define TYPE_CODE 0x00AC
	#ifdef ALT_ID
		#define MODBUS_ID 0x05
	#else
		#define MODBUS_ID 0x08
	#endif
#endif

#ifdef SENSORKIT_ATMOS_R1
	#define MODBUS_ID 0x10
	#define TYPE_CODE 0x00A1
#endif

