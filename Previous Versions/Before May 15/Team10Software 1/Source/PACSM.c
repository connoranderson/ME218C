#define ALL_BITS (0xff<<2)
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_Timer.h"
#include "inc/hw_pwm.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "PACSM.h"
#include "XBee.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define RETRY_TIME 200
#define UNPAIR_TIME 45000

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
//static void SendStatus(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static PACState_t  CurrentState;
static uint32_t CurrentStatus;
static bool PairRequestFlag = false;
																 									 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPACSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
    Connor Anderson, 10/25/2015
****************************************************************************/
bool InitPACSM ( uint8_t Priority )
{
  ES_Event ThisEvent;
	
  MyPriority = Priority;	// Initialize the MyPriority variable with the passed in parameter.	
	
// Set CurrentState to be Waiting4Char
	CurrentState = Unpaired;
	printf("Entering the unpaired state \r\n");

	InitXBee();
// End of InitializeButton (return True)
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostPACSM

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Connor Anderson and Will Roderick, 5/05/2016
****************************************************************************/
bool PostPACSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPACSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   Connor Anderson and Will Roderick, 5/05/2016
****************************************************************************/
ES_Event RunPACSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	
  switch (CurrentState){
    case Unpaired :
			if (ThisEvent.EventType == PAIR_REQUEST) {
				if(PairRequestFlag == false) {
//				InitiatePairing();
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIMER);
					PairRequestFlag = true;
					printf("Requesting pair \r\n");
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){
				ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIMER);
//				InitiatePairing();
				
				
//				ES_Event NewEvent;
//				NewEvent.EventType = PAIR_SUCCESS;
//				PostPACSM(NewEvent); /////////////////////////// CHANGE THIS FOR THE REAL CODE//////////////////////////////////////////////////////
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){
				CurrentState = Unpaired;
				ES_Timer_StopTimer(RETRY_TIMER);
				PairRequestFlag = true;
				printf("Entering the unpaired state \r\n");
			}
			if (ThisEvent.EventType == PAIR_SUCCESS) {
				CurrentState = Paired;
				printf("Entering the paired state \r\n");
				ES_Timer_InitTimer(UNPAIR_TIMER, UNPAIR_TIME);
				ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
				ES_Timer_StopTimer(RETRY_TIMER);
				// Send encryption key
				// 
				PairRequestFlag = false;
			}
			break;
		case Paired :
			if (ThisEvent.EventType == PAIR_SUCCESS) {					///// NOTE THAT THIS USES PAIR SUCCESS 2 DIFFERENT WAYS DEPENDING ON STATE
				// If timer flag set, transmit control packet, clear timer flag, restart 1s and 200ms timers, else set ack flag
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){
				// If the ack flag is set, transmit control packet, clear ack flag, reset 200ms timer, else set timer flag
				
				ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
				ES_Timer_InitTimer(RETRY_TIMER, 800);  /////////////////////////// CHANGE THIS FOR THE REAL CODE//////////////////////////////////////////////////////
				printf("Current Status is : %d \r\n", CurrentStatus);
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){
				CurrentState = Unpaired;
				printf("Entering the unpaired state \r\n");
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == UNPAIR_TIMER){
				CurrentState = Unpaired;
				printf("Entering the unpaired state \r\n");
			}
			if (ThisEvent.EventType == MANUAL_UNPAIR){
				CurrentState = Unpaired;
				printf("Entering the unpaired state \r\n");
			}
      break;
  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/


// Testing status updates
void ChangeStatus(uint32_t NewStatus) {
	CurrentStatus = NewStatus;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


