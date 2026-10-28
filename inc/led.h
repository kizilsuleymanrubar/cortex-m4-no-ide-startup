#ifndef __LED_H__
#define __LED_H__


#include "stm32f411xe.h"

typedef enum
{
    LED_STATE_IDLE = 0,
    LED_STATE_ON, 
    LED_STATE_OFF
}LED_State_t;

void led_init(void);
void led_on(void);
void led_off(void);


#endif
