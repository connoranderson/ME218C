#ifndef _LIFTFAN_H
#define _LIFTFAN_H

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

void InitLiftFan( void);
void TurnOnLiftFan( void);
void TurnOffLiftFan( void);

#endif 
