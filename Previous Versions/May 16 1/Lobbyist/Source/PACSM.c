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
#include "UART.h"
#include "XBee.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define RETRY_TIME 200
#define UNPAIR_TIME 45000

#define PAIR_ARRAY_SIZE 3
#define RED 0
#define BLUE 1




/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitiatePairing(void);
static void InitPackets(void);
static void TransmitControlPacket(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static PACState_t CurrentState;
static bool PairRequestFlag = false;
static bool ACKFlag = false;
static bool TimerFlag = false;
static uint8_t PairDataArray[2];
static uint8_t *PairData = &PairDataArray[0];
static uint8_t ControlPacketArray[5];
static uint8_t *ControlPacket = &ControlPacketArray[0];
static uint8_t DestinationAddress;
static uint8_t Color;

static uint8_t incomingMessageArray[32];
static uint8_t *incomingMessage = &incomingMessageArray[0];
static uint8_t PairByte;
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
	
	
	// TEST DEFINES
printf("RUNNING TEST CODE!!! Just so you know. \r\n");
Color = RED;
DestinationAddress = 0x00;
	
// Set CurrentState to be Waiting4Char
	CurrentState = Unpaired;
	printf("Entering the unpaired state \r\n");

	InitPackets();
	
	InitXBee();
// End of InitializePACSM (return True)
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
					InitiatePairing();
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIMER);
					PairRequestFlag = true;
					printf("Requesting pair \r\n");
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){
				ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIMER);
				InitiatePairing();
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){
				CurrentState = Unpaired;
				ES_Timer_StopTimer(RETRY_TIMER);
				PairRequestFlag = false;
				printf("Entering the unpaired state \r\n");
			}
			if (ThisEvent.EventType == MESSAGE_RECEIVED) {
				incomingMessage = GetMessage();
				PairByte = (*incomingMessage+1);
				if(PairByte&0x03==0x00) {					// Test the error encryption and pair bits
					CurrentState = Paired;
					printf("Entering the paired state \r\n");
					ES_Timer_InitTimer(UNPAIR_TIMER, UNPAIR_TIME);
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);
					// TODO: Send encryption key 																				///////////////////////
					PairRequestFlag = false;
				} else {
					printf("Error in pair or decryption!");
				}
			}
			break;
		case Paired :
			if (ThisEvent.EventType == MESSAGE_RECEIVED) {	
				incomingMessage = GetMessage();
				PairByte = (*incomingMessage+1);
				if(PairByte&0x03==0x00) {					// Test the error encryption and pair bits				
					if(TimerFlag == true) {
						TransmitControlPacket();
						TimerFlag = false;
						ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
						ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);
					} else {
						ACKFlag = true;
					}
				} else {
					printf("Error in pair or decryption!");
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){
				if(ACKFlag == true) {
					TransmitControlPacket();
					ACKFlag = false;
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);
				} else {
					TimerFlag = true;
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){
				CurrentState = Unpaired;
				printf("Entering the unpaired state \r\n");
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == UNPAIR_TIMER){			//// THIS IS TECHNICALLY NOT NEEDED IF WE USE THE SM's FROM THE CHARTS
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


/****************************************************************************
 Function
  InitiatePairing

 Description
	Initates pairing by sending a pairing request to the Lobbyist

****************************************************************************/
static void InitiatePairing(void){ 
	// TODO: Get DestinationAddress  																	////////////////////////
	// TODO: Get Color																							////////////////////////
	PairData[1] = DestinationAddress; // Save destination address						
	PairData[1] = (PairData[1]&BIT7LO)|Color;  // Save color
	TransmitMessage(PairData, PAIR_ARRAY_SIZE,0x00);// uint16_t destAddress) /// SORT OUT WHAT GOES HERE
}

static void TransmitControlPacket(void) { 
	// TODO: Encrypt
	// TODO: Update straight measure (byte 1) of control packet
	// TODO: Update turn measure (byte 2) of control packet
	// TODO: Update special action (byte3) of control packet
	
	ControlPacket[4] = ControlPacket[0]+ControlPacket[1]+ControlPacket[2]+ControlPacket[3];
	TransmitMessage(PairData, PAIR_ARRAY_SIZE,0x00);// uint16_t destAddress) /// SORT OUT WHAT GOES HERE
}

static void InitPackets(void) {
	/*--------------------------- Intialize Data Arrays ------------------------------*/
	PairDataArray[0] = 0x00;	
	PairDataArray[1] = 0x00;	

	ControlPacketArray[0] = 0x02;
	ControlPacketArray[1] = 0x00;
	ControlPacketArray[2] = 0x00;
	ControlPacketArray[3] = 0x00;
	ControlPacketArray[4] = 0x00;
}

/***************************************************************************
 public functions
 ***************************************************************************/


void SetStraight(uint8_t StraightMeasure) {
	ControlPacketArray[1] = StraightMeasure;
}

void SetTurn(uint8_t TurnMeasure) {
	ControlPacketArray[2] = TurnMeasure;
}

void SetSpecialAction(uint8_t SpecialAction) {
	// TODO: IMPLEMENT SPECIAL ACTION
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


