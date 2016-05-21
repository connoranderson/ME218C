#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "ES_Timers.h"
#include "ES_Port.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"

#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "ADMulti.h"
#include "ADService.h"

#define ALL_BITS (0xff<<2)
#define MAX_ANALOG_READ 4095
#define POT_TO_MOTOR_SPEED 41


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint32_t ADResults[2]; // 2 analog read pins
static ADState_t CurrentState;
static uint16_t CurrentMotorSpeed = 50; // Stores the value that the DCService module uses to adjust the motor speed
static uint8_t POT_TO_STEP_SPEED;
static uint16_t currentPotValue = 0;
static uint16_t desiredRPM = 0;

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

uint16_t GetMotorSpeed(void);
uint16_t GetDesiredRPM(void);

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    InitializeDialService
 
 Parameters
 uint8_t : the priorty of this service
 
 Returns
 bool, false if error in initialization, true otherwise
 
 Description
 Saves away the priority, and does any
 other required initialization for this service
 ****************************************************************************/
bool InitializeADService(uint8_t Priority){
// Initialize the MyPriority variable with the passed in parameter
    MyPriority = Priority;
// Initialize PE0 and PE1 as ADC for 2 dials
    ADC_MultiInit(2);
// Set CurrentState to Dial_Idle
    CurrentState = WaitingForTimeout;
	// Initialize a timer to prevent repeated pressing of button
		ES_Timer_InitTimer(AD_TIMER, 20);
	
	
// Return true
    return true;
}

/****************************************************************************
 Function
 PostDialService
 
 Parameters
 EF_Event ThisEvent ,the event to post to the queue
 
 Returns
 bool false if the Enqueue operation failed, true otherwise
 
 Description
 Posts an event to this state machine's queue
 ****************************************************************************/
bool PostADService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
 RunDialService
 
 Parameters
 ES_Event : the event to process
 
 Returns
 ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise
 
 Description
 Process dial input events
 ****************************************************************************/
ES_Event RunADService(ES_Event ThisEvent) {
// Declare a new event ReturnEvent
    ES_Event ReturnEvent;
// Set the EventType of ReturnEvent to ES_NO_EVENT
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

	
		 switch (CurrentState){

			case WaitingForTimeout :
				if (ThisEvent.EventType == ES_TIMEOUT ){
					ADC_MultiRead(ADResults);
					currentPotValue = ADResults[0];
					ES_Timer_InitTimer(AD_TIMER, 20);
				}
      break;
				
  }
		
// Return ReturnEvent
    return ReturnEvent;
}


// Returns a value 0-100 for mapping to duty cycle
uint16_t GetMotorSpeed(void){
	CurrentMotorSpeed = currentPotValue/POT_TO_MOTOR_SPEED; // MAPS to Duty Cycle
	return CurrentMotorSpeed;
	
}
// Returns a value 0-4095 for mapping to RPM in Feedback control
uint16_t GetPotValue(void){
	return currentPotValue;
}
