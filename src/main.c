#if !defined(SEMIHOSTING)
	#include "uart.h"
#endif
#include "led.h"
#include "sensor.h"
#include "systick.h"


#ifdef SEMIHOSTING
	void initialise_monitor_handles(void);
#endif


void SysTick_Handler(void)
{
	temp_sensor_isr();
	led_isr();
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
	#if !defined(SEMIHOSTING)
		uart_init();
	#endif

	while(1)
	{
		temp_sensor_task();
		led_task();
	}
}
