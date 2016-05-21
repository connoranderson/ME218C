#ifndef _PWM_TIVA_
#define _PWM_TIVA_
#include <stdint.h>

void PWM_TIVA_Init(void);
void PWM_TIVA_SetDuty( uint8_t dutyCycle, uint8_t channel);
void PWM_TIVA_SetPeriod( uint16_t reqPeriod, uint8_t group);
void PWM_TIVA_SetFreq( uint16_t reqFreq, uint8_t group);
void PWM_TIVA_SetPulseWidth( uint16_t NewPW, uint8_t channel);

#endif
