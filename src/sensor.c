#include <stdio.h>
#include <stdint.h>
#include "stm32f411xe.h"


#define ADC1EN (1U << 8)
#define ADC_VREF_CH 17U
#define ADC_TEMP_CH 18U


#define CAL_VREFINT (*(const volatile uint16_t *)0x1FFF7A2AUL)
#define CAL_30 (*(const volatile uint16_t *)0x1FFF7A2CUL)  /* ADC @ 30 C  */
#define CAL_110 (*(const volatile uint16_t *)0x1FFF7A2EUL) /* ADC @ 110 C */


typedef struct
{
    double temp_c;
    uint32_t raw_vref;
    uint32_t raw_temp;
    uint8_t volatile isValid;
    uint8_t volatile isr;
} Sensor_t;

static Sensor_t temp_sensor;


static void tmp_sensor_init(void)
{
    /*Enable clock access to the ADC module*/
    RCC->APB2ENR |= ADC1EN;
    (void)RCC->APB2ENR;

    /*Power up internal temperature sensor*/
    ADC->CCR |= ADC_CCR_TSVREFE;

    /*Set conversion sequence length as 1*/
    ADC1->SQR1 = 0;

    /*Set sampling time as 480 cycles*/
    ADC1->SMPR1 |= ADC_SMPR1_SMP17;
    ADC1->SMPR1 |= ADC_SMPR1_SMP18;

    /*Enable ADC module*/
    ADC1->CR2 |= ADC_CR2_ADON;

    /*Wait for ADC and temp sensor stabilization*/
    for (volatile uint32_t i = 0; i < 1000; i++) { }
}


void temp_sensor_task(void)
{
    typedef enum
    {
        STATE_INIT,
        STATE_READ_VREF,
        STATE_WAIT_VREF,
        STATE_READ_SENSOR,
        STATE_WAIT_SENSOR,
        STATE_PROCESS,
        STATE_PRINT
    } State_t;

    // Set initial state
    static State_t state = STATE_INIT;

    switch (state)
    {
        case STATE_INIT:
        {
            tmp_sensor_init();
            state = STATE_READ_VREF;
            break;
        }

        case STATE_READ_VREF:
        {
            ADC1->SQR3 = ADC_VREF_CH;
            ADC1->CR2 |= ADC_CR2_SWSTART;
            state = STATE_WAIT_VREF;
            break;
        }

        case STATE_WAIT_VREF:
        {
            if (!(ADC1->SR & ADC_SR_EOC))
                break;

            temp_sensor.raw_vref = ADC1->DR;
            state = STATE_READ_SENSOR;
            break;
        }

        case STATE_READ_SENSOR:
        {
            ADC1->SQR3 = ADC_TEMP_CH;
            ADC1->CR2 |= ADC_CR2_SWSTART;
            state = STATE_WAIT_SENSOR;
            break;
        }

        case STATE_WAIT_SENSOR:
        {
            if (!(ADC1->SR & ADC_SR_EOC))
                break;

            temp_sensor.raw_temp = ADC1->DR;
            state = STATE_PROCESS;
            break;
        }

        case STATE_PROCESS:
        {
            if (temp_sensor.raw_vref == 0 || temp_sensor.raw_temp == 0 ||
                CAL_VREFINT == 0 || CAL_VREFINT == 0xFFFF || CAL_110 <= CAL_30) {
                state = STATE_READ_VREF;
                break;
            }

            double k = (double)temp_sensor.raw_temp * CAL_VREFINT / (double)temp_sensor.raw_vref;
            temp_sensor.temp_c = 30 + 80. * (k - CAL_30) / (CAL_110 - CAL_30);
            temp_sensor.isValid = 1;
            state = STATE_PRINT;
            break;
        }

        case STATE_PRINT:
        {
            if (temp_sensor.isr) {
                temp_sensor.isValid = 0;
                temp_sensor.isr = 0;
                state = STATE_READ_VREF;

                printf("vref=%lu (cal %u)  temp=%lu (cal30 %u)  T=%.2f C\n",
                    (unsigned long)temp_sensor.raw_vref, (unsigned)CAL_VREFINT,
                    (unsigned long)temp_sensor.raw_temp, (unsigned)CAL_30, temp_sensor.temp_c);
            }

            break;
        }
    }
}


void temp_sensor_isr(void)
{
    if (temp_sensor.isValid)
        temp_sensor.isr = 1;
}
