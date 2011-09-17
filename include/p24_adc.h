#ifndef _PIC24_ADC_H
#define _PIC24_ADC_H

void ProcessADCEvents( unsigned char pin_mask, unsigned int pin_interval_us, 
						unsigned char repeat_count, unsigned int repeat_interval_ms);


#endif
