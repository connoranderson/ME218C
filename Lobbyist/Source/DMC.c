#define ALL_BITS (0xff<<2)
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

#include <string.h>
#include "ES_Timers.h"
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

#include "DMC.h"


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define BIT_TIME 1000
#define RED 0x00
#define BLUE 0x01
#define UNPAIRED 0x00
#define PAIRED 0x01

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t Color = 0x00;
static uint8_t Pairing = 0x00;
static uint8_t BitCount = 0x00;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDMC

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine

****************************************************************************/
bool InitDMC ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;

	// Initialize Port E pin 2
    // Set bit 1 and enable Port E
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4);
    // Set Port E bit 2 to be a digital I/O pins
    HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= GPIO_PIN_2;
    // Make bit 2 on Port E to be an output
    HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) |= GPIO_PIN_2;
	
	
	// Start by clearing the color
	HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  &= ~GPIO_PIN_2;
	
	// set up the short timer for inter-command timings
    ES_ShortTimerInit(MyPriority, SHORT_TIMER_UNUSED );
	
	
  // post the initial transition event
  ThisEvent.EventType = ES_NO_EVENT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostDMC

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
****************************************************************************/
bool PostDMC( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunDMC

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Deals with the high level state transitions of the DMC state machine.
****************************************************************************/
ES_Event RunDMC( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

	if(ThisEvent.EventType == UPDATE_DMC_COLOR) {
		 Color = ThisEvent.EventParam;								// Update the color of the DMC
		printf("Color: %d \r\n", Color);
	}
	if(ThisEvent.EventType == UPDATE_DMC_PAIRING) {
		 Pairing = ThisEvent.EventParam;              // Update the pairing of the DMC
		printf("Pairing: %d \r\n", Pairing);
	}
	if(ThisEvent.EventType == UPDATE_DMC) {
		HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  |= GPIO_PIN_2;  // Set the start bit and bit time of the DMC
    ES_ShortTimerStart(TIMER_A,BIT_TIME);	// Start the short timer for 1ms (These are better resolution than regular timers)
	}
	if(ThisEvent.EventType == ES_TIMEOUT || ThisEvent.EventType == ES_SHORT_TIMEOUT) {
		// Write the correct pairing and color bits to the DMC
		if(BitCount == 0x00) {
			if(Pairing == UNPAIRED) {
				 HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  &= ~GPIO_PIN_2;
			} else if(Pairing == PAIRED) {
				 HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  |= GPIO_PIN_2;
				//printf("P");				
			}
			BitCount++;
          ES_ShortTimerStart(TIMER_A,BIT_TIME); // Start the short timer for 1ms (These are better resolution than regular timers)
			
		} else if (BitCount == 0x01) {
			 if(Color == RED) {
				 HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  &= ~GPIO_PIN_2;
			 } else if(Color == BLUE) {
				 HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  |= GPIO_PIN_2;
				 //printf("B");
			 }
			 BitCount++;
          ES_ShortTimerStart(TIMER_A,BIT_TIME); // Start the short timer for 1ms (These are better resolution than regular timers)

		} else {
			 BitCount = 0x00;
				HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  &= ~GPIO_PIN_2;
				printf("DMC Done \r\n");
		}
	}
  return ReturnEvent;
}

///****************************************************************************
// Function
//	SetColor

// Description
//	Sets the color of the Lobbyist

//****************************************************************************/
//bool SetColor(uint8_t Color){ 
//	if(Color > 0) {
//		HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  |= GPIO_PIN_2;
//	} else {
//		HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS))  &= ~GPIO_PIN_2;
//	}
//	return true;
//}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

