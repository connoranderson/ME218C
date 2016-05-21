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
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "DMC.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable






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

//#include "UART.h"
#include "XBee.h"
#include "Encryption.h"
#include "Steering.h"
#include "LiftFan.h"
#include "DMC.h"
#include "Badge.h"

/*----------------------------- Module Defines ----------------------------*/
#define ALL_BITS (0xff<<2)
#define ONE_SEC 1000
#define RETRY_TIME 200
#define UNPAIR_TIME ONE_SEC

#define PAIRED 0x01
#define UNPAIRED 0x00
#define DECRYPT_FAILURE 0x03

#define PAIR_REQUEST_HEADER 0x00
#define CNTRL_REQUEST_HEADER 0x02
#define KEY_SEND_HEADER 0x01

#define STATUS_PACKET_SIZE 3 
#define CNTRL_CHECKSUM 0x04 
//#define DESIRED_XBEE 0x2189

#define INCOMING_PACKET_API_IDENTIFIER 0x81

#define LEFT 0
#define RIGHT 1
#define OFF 0x00

/*---------------------------- Module Functions ---------------------------*/
static void InitPackets(void);
static void TransmitStatusPacket(uint8_t PairedStatus);
static bool CheckLobbyistNumber(void);
static void ExecuteControl(void);
static uint8_t GetCheckSum(uint8_t *incomingData);
static bool UpdateColor(uint8_t *incomingData);

/*---------------------------- Module Variables ---------------------------*/
static LobbyistState_t CurrentState;
static uint8_t MyPriority;
static uint8_t PairTimeLeft;
static uint8_t StatusPacketArray[STATUS_PACKET_SIZE];
static uint8_t *StatusPacket = &StatusPacketArray[0];
static uint16_t DestinationAddress = 0x2089;
static uint8_t incomingmessagearr[8];
static uint8_t *incomingMessage = &incomingmessagearr[0];
static uint8_t decryptedMessageArr[8];
static uint8_t *decryptedMessage = &decryptedMessageArr[0];
static uint8_t LobbyistNumber = 0x0D; // Our ID which the PAC uses to pair with us
                                      
static uint16_t pairedAddress; // Address of the PAC we are paired to

static uint8_t command = 0;
static uint8_t *lastCommandFrame = &command;
static uint8_t encryptedChecksum;

static uint8_t Color = 0x00;

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
ES_Event RunLobbyistSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  return ReturnEvent;
}

















/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDMC

 Description
     Initializes the pin for the DMC
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
bool InitDMC (void)
{

	
	return true;
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

