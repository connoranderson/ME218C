/****************************************************************************
 Module
   LobbyistSM.c

 Description
   This is the high level state machine for the Lobbyist.

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "LobbyistSM.h"
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
#include "LobbyistSM.h"

#include "UART.h"
#include "XBee.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
/*---------------------------- Module Functions ---------------------------*/

/*---------------------------- Module Variables ---------------------------*/
static LobbyistState_t CurrentState;
static uint8_t MyPriority;
static uint8_t PairTimeLeft = 45;



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLobbyistSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine

****************************************************************************/
bool InitLobbyistSM ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = InitPState;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
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
     PostLobbyistSM

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
****************************************************************************/
bool PostLobbyistSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLobbyistSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Deals with the high level state transitions of the Lobbyist state machine.
****************************************************************************/
ES_Event RunLobbyistSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case InitPState :       // If current state is initial Psedudo State
        if ( ThisEvent.EventType == ES_INIT ) // only respond to ES_Init
        {
            //Prep the timer hardware.
            InitXbee(); //Initialize the comms.
            //Initialize the DMC.
            CurrentState = Unpaired;
        }
    break;

    case Unpaired :
      switch ( ThisEvent.EventType )
      {
        //If the event is a pair request, change state to pairing and wait for the key.
        case PAIR_REQUEST:
				printf("\n\r\nPair request received in unpaired state.");
        //Start the 45s timer
        //Start the 1s timer
        //Start the lift fan
        //Update DMC
        //respond to the status request.
        CurrentState = Pairing;

        break;     
        default :
            ; 
      }
    break;

    case Pairing:
      switch(ThisEvent.EventType){
        case KEY_RECEIVED:
				printf("\n\r\nKey received in pairing state.");
        //Save the key
        //Restart the 1s timer
        //transmit status with the pairing bit set.
        CurrentState = Paired;
        break;

        case ES_TIMEOUT: //1s timer
				printf("\n\r\n1ms timeout in pairing state.");
        //Turn off the lift fan
        //Update DMC to avaiable for pairing
        //Disable the 45s pairing timer
        //Disable the 1s transmit timeout timer
        //Transmit status with the pairing bit cleared.
        CurrentState = Unpaired;
        break;
      }
    break;

    case Paired:
      switch(ThisEvent.EventType){
        case COMMAND_RECEIVED:
				printf("\n\r\nCommand received in the paired state.");
        //Restart the command timer.
        //Execute the command.
        //Transmit the status with pairing bit set.
        break;

        case ES_TIMEOUT: //Either the 1s or 45s timer
          switch(ThisEvent.EventParam){
            case PAIRING_TIMER:
              PairTimeLeft--;
              printf("\n\r\nTime until unpair: %ds", PairTimeLeft);
              if(PairTimeLeft == 0){
                CurrentState = Unpaired;
                printf("\n\r\nUnpairing.");
              } else {
                //Reset the pair timer.
              }
          }
				printf("\n\r\nTimeout in the paired state.");
        //Turn off the lift fan
        //Update DMC to avaiable for pairing
        //Disable the 45s pairing timer
        //Disable the 1s transmit timeout timer
        //Transmit status with the pairing bit cleared.
        CurrentState = Unpaired;
        break;
      }

    break;

  default :
    break;
  }

  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryLobbyistSM

 Parameters
     None

 Returns
     LobbyistState_t The current state of the Lobbyist state machine

 Description
     returns the current state of the Lobbyist state machine
****************************************************************************/
LobbyistState_t QueryLobbyistSM ( void )
{
   return(CurrentState);
}

/***************************************************************************
 private functions
 ***************************************************************************/

