/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   This is the sample for writing event checkers along with the event 
   checkers used in the basic framework test harness.

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.
   
 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 13:36 jec     initial version
****************************************************************************/

#include "ADMulti.h"

#define ALL_BITS (0xff<<2)
#define BUTTON_PRESSED 0
#define BUTTON_NOT_PRESSED 1
#define HUMAN_DETECTED true
#define ACCEL_THRESHOLD 2200

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function 
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header & 
// actual functionsdefinition
#include "EventCheckers.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"


// This is the event checking function sample. It is not intended to be 
// included in the module. It is only here as a sample to guide you in writing
// your own event checkers
#if 0
/****************************************************************************
 Function
   Check4Lock
 Parameters
   None
 Returns
   bool: true if a new event was detected
 Description
   Sample event checker grabbed from the simple lock state machine example
 Notes
   will not compile, sample only
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
//bool Check4Lock(void)
//{
//  static uint8_t LastPinState = 0;
//  uint8_t CurrentPinState;
//  bool ReturnVal = false;
//  
//  CurrentPinState =  LOCK_PIN;
//  // check for pin high AND different from last time
//  // do the check for difference first so that you don't bother with a test
//  // of a port/variable that is not going to matter, since it hasn't changed
//  if ( (CurrentPinState != LastPinState) &&
//       (CurrentPinState == LOCK_PIN_HI) )
//  {                     // event detected, so post detected event
//    ES_Event ThisEvent;
//    ThisEvent.EventType = ES_LOCK;
//    ThisEvent.EventParam = 1;
//    // this could be any of the service post function, ES_PostListx or 
//    // ES_PostAll functions
//    ES_PostList01(ThisEvent); 
//    ReturnVal = true;
//  }
//  LastPinState = CurrentPinState; // update the state for next time

//  return ReturnVal;
//}
#endif

/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so, 
   retrieves the key and posts an ES_NewKey event to TestHarnessService0
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker 
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  if ( IsNewKeyReady() ) // new key waiting?
  {
    ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    // test distribution list functionality by sending the 'L' key out via
    // a distribution list.
    if ( ThisEvent.EventParam == 'L'){
//      ES_PostList00( ThisEvent );
    }else{   // otherwise post to Service 0 for processing
      PostTestHarnessService0( ThisEvent );
    }
    return true;
  }
  return false;
}


bool Check4ButtonPress(void)
{
 static uint8_t LastPinState = 0;
 uint8_t CurrentPinState;
 bool ReturnVal = false;
	
 CurrentPinState =  HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT4HI; // Read button input pin F4
 // check for pin high AND different from last time
 // do the check for difference first so that you don't bother with a test
 // of a port/variable that is not going to matter, since it hasn't changed
 if ( (CurrentPinState != LastPinState) &&
      (CurrentPinState == BUTTON_PRESSED) )
 { // event detected, so post detected event
   ES_Event ThisEvent;
   ThisEvent.EventType = ES_BUTTON_PRESS;
   ThisEvent.EventParam = 0;
   // this could be any of the service post function, ES_PostListx or 
   // ES_PostAll functions
   PostButtonService(ThisEvent); 
	 
   ReturnVal = true;
 }
 LastPinState = CurrentPinState; // update the state for next time

 return ReturnVal;
}

bool Check4InitPairing(void)
{
	 bool ReturnVal = false;
	 static uint16_t LastADReading = 0;
// Read dial positions from analog pins and store them in ADResults
   uint32_t ADResults[1]; // 1 analog read pin (PE0)
   ADC_MultiRead(ADResults);
// Save the current dial position reading to ADReading
   uint16_t CurrentADReading = (uint16_t) ADResults[0]; // stores the AD reading
	
	bool CurrentCapState =  HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI; // Read pin F3
// printf("AD Reading: %d \r\n", CurrentADReading);
//printf("AD Reading: %d \r\n", CurrentADReading);
// check for accel above threshold AND last accel reading lower than threshold
// do the check for difference first so that you don't bother with a test
// of a port/variable that is not going to matter, since it hasn't changed
 if ( (CurrentADReading >= ACCEL_THRESHOLD) && (LastADReading <= ACCEL_THRESHOLD) && CurrentCapState == HUMAN_DETECTED) { // event detected, so post detected event
			ES_Event NewEvent;
			NewEvent.EventType = PAIR_REQUEST;
			PostPACSM(NewEvent);
			//printf("Pair CMD sent\r\n");
			//printf("AD Reading: %d \r\n", CurrentADReading);
			ReturnVal = true;
 }
 LastADReading = CurrentADReading; // update the state for next time

 return ReturnVal;
}

