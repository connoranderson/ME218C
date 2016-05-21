/****************************************************************************
 Module
   DCMotor.c

 Revision
   2.0.1

 Description
   This is a template file for implementing state machines.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
// Basic includes for a program using the Events and Services Framework
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





/*----------------------------- Module Defines ----------------------------*/
// define constants for the states for this machine
// and any other local defines
#define ALL_BITS (0xff<<2)



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine, things like during
   functions, entry & exit functions.They should be functions relevant to the
   behavior of this state machine
*/


/*---------------------------- Module Variables ---------------------------*/


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    InitLiftFan

 Description
   Turns on Lift Fan

****************************************************************************/

void InitLiftFan( void){
	// Initialize Port B pin 2 
    // enable Port B
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
    // Set Port B bits 2 to be a digital I/O pin
    HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_2) ;
    // Make bits 2 on Port B to be output
    HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_2) ;
	
	// Stop the motors before proceeding
    // Set the necessary pins to LO on the lift fan
    HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT2LO;
	
}
/****************************************************************************
 Function
    StartLiftFan

 Description
   Turns on Lift Fan

****************************************************************************/

void TurnOnLiftFan( void){
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) |= BIT2HI;
}

/****************************************************************************
 Function
    StartLiftFan

 Description
   Turns on Lift Fan

****************************************************************************/

void TurnOffLiftFan( void){
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) &= BIT2LO;
}