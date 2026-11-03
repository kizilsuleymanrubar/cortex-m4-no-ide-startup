#include "stm32f411xe.h"


void systick_init(void)
{
	SysTick->LOAD = 16000000 - 1U;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x7;
}
