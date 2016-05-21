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
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "Button.h"
#include "UART.h"
#include "XBee.h"


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define REGISTER_HIGH BIT1HI
#define REGISTER_LOW 0x00
#define NUM_INIT_STEPS 10
#define USE_4_BIT_WRITE 0x8000

#define BUTTON_PRESSED 0
#define BUTTON_NOT_PRESSED 1
#define DEBOUNCE_TIMER 2


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static ButtonState_t CurrentState;

static bool LastButtonState_1;
static bool LastButtonState_2;
static bool LastButtonState_3;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLCDService

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
bool InitButtonService ( uint8_t Priority )
{
	puts("Button Service Running");
  ES_Event ThisEvent;
  
  MyPriority = Priority;	// Initialize the MyPriority variable with the passed in parameter.


	// Initialize the port line to monitor the button
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;	// Initialize port F
		
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != SYSCTL_PRGPIO_R5);	// Wait until initialized
	 
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_4);	// Initialize F4 for button use
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) &= (BIT4LO);
	
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_3);	// Initialize F3 for output
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= (GPIO_PIN_3);
	
	HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) &= BIT3LO; // Set F3 LOW
	
	// Sample the button port pin and use it to initialize LastButtonState
	LastButtonState_1 = HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT4HI;

		
// Set CurrentState to be DEBOUNCING
	CurrentState = Debouncing;

// Initialize a timer to prevent repeated pressing of button
  ES_Timer_InitTimer(DebounceTimer, HALF_SEC);

// End of InitializeButton (return True)
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
     PostLCDService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Connor Anderson, 10/25/2015
****************************************************************************/
bool PostButtonService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLCDService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   Connor Anderson, 10/25/2015
****************************************************************************/
ES_Event RunButtonService( ES_Event ThisEvent )
{
	
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  uint16_t delayTime;
  
  switch (CurrentState){

    case Debouncing :
      if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == 14){
				CurrentState = WaitingForPress;
      }
      break;
    case WaitingForPress :
      if( ThisEvent.EventType == ES_BUTTON_PRESS ){ // button press detected
						puts("Button Pressed");
					uint8_t firstTransmit[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
					uint8_t *p = &firstTransmit[0]; 
					TransmitMessage(p, 10, 0xffff);
      		ES_Timer_InitTimer(DebounceTimer, HALF_SEC); // start new debounce timer
	       	CurrentState = Debouncing; // switch to debouncing
        } 
      break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/



/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

