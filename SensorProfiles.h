/* Defenitions for Sensor Boards / Customisations */

/* Register Offsets
 * Analog Inputs:
 *		Single Sample		0x100
 * 		Multi Sample  	 	0x200 (max 16 samples per Address, 16 channels)
 *				ex: 	0x200 to 0x20F, 0x210 to 0x21F
 *	Digital Inputs
 *		i2c Registers			0x300
 *		Frequency				0x400
 *		CTMU					0x500
 */
#if defined (SENSORKIT_EKO_TEST)
	//
	#define MODBUS_ID 0x11
	#define SUPPORT_MB04 // supporting function code 04
	
	
#elif defined (SENSORKIT_DC3240_R1)
	//
#elif defined (SENSORKIT_ATMOS_R1)
	//
#endif

