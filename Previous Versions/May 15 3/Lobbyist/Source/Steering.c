/****************************************************************************
 Module
   Steering.c


 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 5/12/16				Will		First pass
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/


/* Hardware includes.
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_PWM.h"
#include "inc/hw_Timer.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "bitdefs.h"

#include "Steering.h"
#include "PWM.h"


/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines

#define FORWARD_RIGHT BIT0HI
#define FORWARD_LEFT BIT1HI
#define BACKWARD_RIGHT BIT0LO
#define BACKWARD_LEFT BIT1LO
#define STRAIGHT_CONVERSION 0.39
#define TURN_CONVERSION 0.20
#define MAX_DUTY 50
#define FULL_DUTY 100
#define ALL_BITS (0xff<<2)                       

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions */
static bool UpdateStraight(int8_t StraightMeasure);
static bool UpdateTurning(int8_t TurnMeasure);
static void WriteToMotors(int8_t driveSignal_LeftMotor,int8_t driveSignal_RightMotor);

/*---------------------------- Module Variables ---------------------------*/
static int8_t DutyCycleLeftMotor = 0;
static int8_t DutyCycleRightMotor = 0;

/****************************************************************************
 Function
     InitSteering

 Description
     Initializes the steering hardware

****************************************************************************/
void InitSteering(void){
	PWMInit(); 															// Initialize the PWM
	
	// Initialize Port B pins 0 and 1 - the pins associated with the motor as digital outputs
    // Set bit 1 and enable Port B
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
    // Set Port B bits 0, 1 to be a digital I/O pins
    HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1) ;
    // Make bits 0, 1 on Port B to be outputs
    HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_0 | GPIO_PIN_1) ;
	
	// Stop the motors before proceeding
    // Set the necessary pins to LO on the left motor and LO on the right motor
    HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT1LO;
    HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT0LO;
    // Set the duty cycle to 0 for both motors
    SetDutyCycleLeftMotor(0);
    SetDutyCycleRightMotor(0);
	
	printf("\r\n Steering initialized");
}


/****************************************************************************
 Function
     UpdateSteering

 Description
     Updates the steering

****************************************************************************/
void UpdateSteering(int8_t StraightMeasure, int8_t TurnMeasure){
	// Reset the left and right motors
	DutyCycleLeftMotor = 0;
	DutyCycleRightMotor = 0;
	
	// Call the appropriate functions to set the duty cycles of both motors
	UpdateStraight(StraightMeasure);
	UpdateTurning(TurnMeasure);
	
	// Set the correct bits to move forward or backward
	if(DutyCycleLeftMotor > 0) {
		// Clear the necessary bit to move forward with the left motor
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= FORWARD_LEFT;
	} else {
		// Set the necessary bit to move backward with the left motor
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BACKWARD_LEFT;
	}
	if(DutyCycleRightMotor > 0) {
		// Clear the necessary bit to move forward with the right motor
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= FORWARD_RIGHT;
	} else {
		// Set the necessary bit to move backward with the right motor
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BACKWARD_RIGHT;
	}
	
	// Ensure that the duty cycles do not pass the maximum value
	if(DutyCycleLeftMotor > MAX_DUTY) {
		DutyCycleLeftMotor = MAX_DUTY;
	}
	if(DutyCycleRightMotor > MAX_DUTY) {
		DutyCycleRightMotor = MAX_DUTY;
	}
	// Write these duty cycles to the motors	
	WriteToMotors(DutyCycleLeftMotor,DutyCycleRightMotor);
}

/****************************************************************************
 Function
     UpdateStraight

 Description
     Updates the forward/backward speed

****************************************************************************/
static bool UpdateStraight(int8_t StraightMeasure) {
	// Straight measure ranges from -128 to 127
	int8_t StraightDuty = StraightMeasure*STRAIGHT_CONVERSION;
	DutyCycleLeftMotor += StraightDuty;
	DutyCycleRightMotor += StraightDuty;
	
	return true;
}


/****************************************************************************
 Function
     UpdateTurning

 Description
     Updates the left/right speed

****************************************************************************/
static bool UpdateTurning(int8_t TurnMeasure) {
	// Turn measure ranges from -128 to 127
	int8_t TurnDuty = TurnMeasure*TURN_CONVERSION;
	DutyCycleLeftMotor += TurnDuty;
	DutyCycleRightMotor -= TurnDuty;
	return true;
}



/****************************************************************************
 Function
     WriteToMotors

 Description
     Handles the setting of PWM to each individual motor, based on motor direction pins
		PB0 and PB1

****************************************************************************/
static void WriteToMotors(int8_t driveSignal_LeftMotor,int8_t driveSignal_RightMotor)
{
		uint8_t rightMotorStatus = HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS))&BIT0HI; // Check status of right motor
		uint8_t leftMotorStatus = HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS))&BIT1HI; // Check status of left motor
		
	// Set right motor speed
	if(rightMotorStatus == 0x00){ // Going backwards
		SetDutyCycleRightMotor(FULL_DUTY  + driveSignal_RightMotor);   // If going backwards, drive signal will be negative. Subtract from max duty for backwards PWM.
		printf("\r\n Duty Cycle Right Motor (Backwards): %d", FULL_DUTY  + driveSignal_RightMotor);
	}else{
		SetDutyCycleRightMotor(driveSignal_RightMotor);
		printf("\r\n Duty Cycle Right Motor (Forward): %d", driveSignal_RightMotor);
	}
	
	// Set left motor speed
	if(leftMotorStatus == 0x00){ // Going backwards
			SetDutyCycleLeftMotor(FULL_DUTY + driveSignal_LeftMotor);     // If going backwards, drive signal will be negative. Subtract from max duty for backwards PWM.
			printf("\r\n Duty Cycle Left Motor (Backwards): %d", FULL_DUTY + driveSignal_LeftMotor);
	}else{
		SetDutyCycleLeftMotor(driveSignal_LeftMotor);
		printf("\r\n Duty Cycle Left Motor (Forward): %d", driveSignal_LeftMotor);
	} 
		
}

