#include "stm32f411xe.h"


#define GPIOAEN (1U << 0)
#define LED_BS5 (1U << 5)  /*Bit Set Pin 5*/
#define LED_BR5 (1U << 21) /*Bit Reset Pin 5*/


typedef enum
{
    LED_STATE_IDLE = 0,
    LED_STATE_ON,
    LED_STATE_OFF
} LED_State_t;


static volatile LED_State_t current_state = LED_STATE_IDLE;
static volatile LED_State_t next_state = LED_STATE_ON;


void led_init(void)
{
    /*Enable clock access to GPIOA*/
    RCC->AHB1ENR |= GPIOAEN;
    /*Set PA5 mode to output mode*/
    GPIOA->MODER |= (1U << 10);
    GPIOA->MODER &= ~(1U << 11);
}


static void led_on(void)
{
    /*Set PA5 high*/
    GPIOA->BSRR |= LED_BS5;
}


static void led_off(void)
{
    /*Set PA5 low*/
    GPIOA->BSRR |= LED_BR5;
}


void led_task(void)
{
    switch (current_state)
    {
        case LED_STATE_IDLE:
            break;
        case LED_STATE_OFF:
        {
            led_off();
            next_state = LED_STATE_ON;
            current_state = LED_STATE_IDLE;
            break;
        }
        case LED_STATE_ON:
        {
            led_on();
            next_state = LED_STATE_OFF;
            current_state = LED_STATE_IDLE;
            break;
        }
    }
}


void led_isr(void)
{
    if (current_state == LED_STATE_IDLE)
        current_state = next_state;
}

