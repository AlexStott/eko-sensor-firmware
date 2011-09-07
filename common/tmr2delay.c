#include "board/p24_board.h"

void delay_ms( unsigned int ms )
{
	unsigned int ticks;
	ticks = (ms*15);
	T2CON = 0x0000;
	TMR2 = 0;
	T2CON = 0x8030;
	while (TMR2 < ticks );
}

void delay_us( unsigned int us )
{
	unsigned int ticks;
	ticks = (us/64);
	T2CON = 0x0000;
	TMR2 = 0;
	T2CON = 0x8030;
	while (TMR2 < (ticks) );
}
