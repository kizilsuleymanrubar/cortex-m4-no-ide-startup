#ifndef NOSTDLIB
	#include <stdio.h>
#endif
#include "led.h"
#include "uart.h"


#ifdef SEMIHOSTING
	void initialise_monitor_handles(void);
#endif


static volatile LED_State_t current_state = LED_STATE_IDLE;
static volatile LED_State_t next_state = LED_STATE_ON;


void systick_init(void)
{
	SysTick->LOAD = 16000000 - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x7;
}

void SysTick_Handler(void)
{
	current_state = next_state;
}

int main(void)
{
	// Initialize the semihosting feature
	#ifdef SEMIHOSTING
		initialise_monitor_handles();
	#endif

	// Initialize LED
	led_init();

	// Initialize Systick
	systick_init();

	//Initialize UART
	uart_init();

	while(1)
	{
		switch (current_state)
		{
			case LED_STATE_IDLE:
				break;
			case LED_STATE_OFF:
			{
				led_off();
				#ifndef NOSTDLIB
					printf("LED Status = OFF\n");
				#endif
				current_state = LED_STATE_IDLE;
				next_state = LED_STATE_ON;
				break;
			}
			case LED_STATE_ON:
			{
				led_on();
				#ifndef NOSTDLIB
					printf("LED Status = ON\n");
				#endif
				current_state = LED_STATE_IDLE;
				next_state = LED_STATE_OFF;
				break;
			}

		}
	}
}
