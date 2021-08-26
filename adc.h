#ifndef _ADC_H_
#define _ADC_H_

//fafaf
#include "gd32e230.h"
#include "timer.h"
#include "gpio.h"

void adc_init(void);
float Get_12V(void);
float Get_5V(void);
uint8_t WaitFor12V_InValid(void);

#endif