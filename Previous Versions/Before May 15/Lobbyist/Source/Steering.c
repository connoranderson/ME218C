/*----------------------------- Include Files -----------------------------*/
/* include header files for the framework and this service
 */
// the common headers for C99 types
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "BITDEFS.H"
#include "PWM8Tiva.h"
#include "Steering.h"

#include "driverlib/sysctl.h"




/*------------------------------ Module Defines --------------------------------*/
#define LEFT 0
#define RIGHT 1
#define CORRECTION_FACTOR 10 // Max: 39
#define CENTER 2000

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.
 */
static uint16_t Spd2PWLeftMotor(int8_t Spd);
static uint16_t Spd2PWRightMotor(int8_t Spd);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static const uint16_t reqPeriod_motors = 25000;
static uint16_t CurrentPWLeftMotor = 2000;
static uint16_t CurrentPWRightMotor = 2000;

/***************************************************************************
 public functions
 ***************************************************************************/
void Steering_HWInit(void){
		PWM8_TIVA_Init();	// Initialize the PWM
// set frequency for arm servos (group 0, channel 0 & 1, PB6 & PB7)              
    PWM8_TIVA_SetPeriod(reqPeriod_motors,0);
// set initial PW's
	  PWM8_TIVA_SetPulseWidth(CENTER,LEFT);
	  PWM8_TIVA_SetPulseWidth(CENTER,RIGHT);	
}

void Steering_Write(uint16_t NewSpd, uint8_t channel){
// declare a local variable NewPW to keep track of the pulse width
    uint16_t NewPW;
// if the input channel is channel 0 (Arm 1 servo)
    if (channel == LEFT) {
// convert angular position NewPos to the pulse width for that position associated with the arm 1 servo and set NewPW to that value
        NewPW = Spd2PWLeftMotor(NewSpd);
// else if the input channel is channel 1 (Arm 2 servo)
    } else if (channel == RIGHT) {
// convert angular position NewPos to the pulse width for that position associated with the arm 2 servo and set NewPW to that value
        NewPW = Spd2PWRightMotor(NewSpd);
// else if the input channel is channel 3 (timer servo)
    }
// set duty cycle of the channel to NewPos
    PWM8_TIVA_SetPulseWidth(NewPW,channel);
}

/***************************************************************************
 private functions
 ***************************************************************************/

static uint16_t Spd2PWLeftMotor(int8_t Spd) {
    // Pos: -128 to 127
    // PW: 10000-20000
// declare a local variable NewPW to keep track of the pulse width
    uint16_t PW;
// convert the input speed to the pulse width for that position associated with the left motor and set PW to that value	
	printf("Speed: %d \r\n", Spd);
	if(Spd < 0 && CurrentPWLeftMotor > 800) {
		CurrentPWLeftMotor -= 10;
	} else if (Spd > 0 && CurrentPWLeftMotor < 2500) {
		CurrentPWLeftMotor += 10;
	}
	
      PW =  CurrentPWLeftMotor;
 //   PW = (uint16_t) (15000 + Spd*CORRECTION_FACTOR);
	
// return PW
    return PW;
}

static uint16_t Spd2PWRightMotor(int8_t Spd) {
	  // Pos: -128 to 127
    // PW: 10000-20000
// declare a local variable NewPW to keep track of the pulse width
    uint16_t PW;
// convert the input speed to the pulse width for that position associated with the left motor and set PW to that value
	if(Spd < 0 && CurrentPWRightMotor > 800) {
		CurrentPWRightMotor -= 10;
	} else if (Spd > 0 && CurrentPWRightMotor < 2500) {
		CurrentPWRightMotor += 10;
	}

       PW = CurrentPWRightMotor;
//    PW = (uint16_t) (15000 + Spd*CORRECTION_FACTOR);
	
// return PW
    return PW;
}
