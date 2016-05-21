#ifndef _PWM_H
#define _PWM_H

/****************************************************************************
 Module
     PWM.h
 Description
     header file to support use of PWM on the Tiva
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 2/1/16         Will    first pass
*****************************************************************************/
#include <stdint.h>

void PWMInit(void);
void SetDutyCycleLeftMotor(uint8_t DutyCycleLeftMotor);
void SetDutyCycleRightMotor(uint8_t DutyCycleRightMotor);

#endif //_PWM_H
