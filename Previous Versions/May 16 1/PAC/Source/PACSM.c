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

#include "ADMulti.h"

#include "PACSM.h"
#include "XBee.h"
#include "Encryption.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 999
#define RETRY_TIME 200
#define UNPAIR_TIME 44999													/////////// FIX THIS?

#define PAIR_ARRAY_SIZE 2
#define CONTROL_ARRAY_SIZE 5
#define ENCRYPTION_KEY_PACKET_SIZE 33

#define	PAIR_REQ_HEADER 0x00
#define CONTROL_HEADER 0x02
#define ENCRYPTION_HEADER 0x01

#define RED 0x00
#define BLUE 0x80

#define BROADCAST 0xFFFF
//#define DESIRED_XBEE 0x2089

#define CORRECT_PAIR_STATUS 0x01
#define TX_SIZE 7

#define INCOMING_PACKET_API_IDENTIFIER	0x81

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitPackets(void);
static void InitSensors(void);
static void InitiatePairing(void);
static void TransmitControlPacket(void);
static void TransmitEncryptionKey(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static PACState_t CurrentState;
static bool PairRequestFlag = false;
static bool ACKFlag = false;
static bool TimerFlag = false;
static uint8_t PairDataArray[PAIR_ARRAY_SIZE];
static uint8_t *PairData = &PairDataArray[0];
static uint8_t ControlPacketArray[CONTROL_ARRAY_SIZE];
static uint8_t *ControlPacket = &ControlPacketArray[0];
static uint8_t EncryptionKeyPacketArray[ENCRYPTION_KEY_PACKET_SIZE];
static uint8_t *EncryptionKeyPacket = &EncryptionKeyPacketArray[0];

static uint16_t DestinationAddress = 0xFF;
static uint8_t Color;
static uint8_t LobbyistNumber;


static uint8_t incomingMessageArray[32];
static uint8_t *incomingMessage = &incomingMessageArray[0];
static uint8_t PairByte;

static bool ControlByte = false;


static uint16_t sender;																									////////////// TAKE OUT
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
	
// Set CurrentState to be Waiting4Char
	CurrentState = Unpaired;
	printf("Entering the unpaired state \r\n");

	InitSensors();
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
				if(PairRequestFlag == false) {										// If we are not already pairing
					LobbyistNumber = 0x0D;///	///////////////////////////////////////////////////////////////////////////// Check pin to update number and color being sent
					InitPackets();																						//////////////// DO WE NEED THIS?
					InitiatePairing();															// Initiate pairing
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);   // Start 1s timer
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);    // Start 200ms timer
					PairRequestFlag = true;													// Set the pair request flag to true
					printf("Requesting pair \r\n");
				}
				ControlByte = false;																															/////////////////////////////
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){	// If we did not successfully pair after 200ms
				printf("Resending message\r\n");
				ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);			// Reset the 200ms timer
				InitiatePairing();																// Reinitiate pairing
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){ // If the pairing did not succeed after 1s
				CurrentState = Unpaired;													// Set the current state to unpaired
				ES_Timer_StopTimer(RETRY_TIMER);									// Stop the retry timer
				PairRequestFlag = false;													// Indicate that the PAC is not currently attempting to pair
				printf("Entering the unpaired state \r\n");
			}
			sender = GetSenderAddress();
			DestinationAddress = sender;
			if (ThisEvent.EventType == MESSAGE_RECEIVED && sender == DestinationAddress && PairRequestFlag == true && GetApiIdentifier() == INCOMING_PACKET_API_IDENTIFIER) {
				incomingMessage = GetMessageData();
				PairByte = *(incomingMessage+1);
				printf("Pair Byte: %d \r\n", PairByte);
				if(PairByte==CORRECT_PAIR_STATUS) {					// Test the error encryption and pair bits
					CurrentState = Paired;
					printf("Entering the paired state \r\n");
					ES_Timer_InitTimer(UNPAIR_TIMER, UNPAIR_TIME);
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);
					TransmitEncryptionKey(); // TODO: Send encryption key 																				///////////////////////
					PairRequestFlag = false;
					TimerFlag = false;
					ACKFlag = false;
				} //else {
					//printf("Error in pair or decryption in unpaired \r\n");
				//}
			}
			break;
			
			
		case Paired :
			sender = GetSenderAddress();																		// Get the sender address
			if (ThisEvent.EventType == MESSAGE_RECEIVED && sender == DestinationAddress) {	// Check if the message was received from the correct sender
				incomingMessage = GetMessageData();														// Save the incoming message data
				PairByte = *(incomingMessage+1);															// Save the pair byte
				//printf("Pair Byte: %d \r\n", PairByte);
				if(PairByte==CORRECT_PAIR_STATUS) {														// Test the error encryption and pair bits				
						//} else {
					ACKFlag = true;																						// Set the ACKFlag to indicate that the PAC is paired
					if (ControlByte == true) {
						RotateCipher();
					//		printf("Rotating Key \r\n");
					} else {																											///////////////////////////////////// SORT THIIS OUT
						ControlByte = true;
					//		printf("Set CTL Byte \r\n");
					}
					if(TimerFlag == true) {																			// If the timer has already timed out
						printf("TX CTL PKT. TMR Flag reset \r\n");
// This code should make the transmission slightly faster
						//TransmitControlPacket();																	// Transmit a control packet
						//ACKFlag = false;
						TimerFlag = false;																				// Set the timer flag to false
						ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);							// Reset the 200ms
					}
					ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC);							// Reset the 1s timer
						//printf("ACK \r\n");
				} else {
					printf("Error in pair or decryption in paired \r\n");
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == RETRY_TIMER){ // If the resend timer (200ms) timed out
				//printf("200ms timeout \r\n");
				if(ACKFlag == true) {																					// If the PAC is paired
					printf("TX CTL PKT \r\n");
					TransmitControlPacket();																		// Transmit a control packet
					ACKFlag = false;																						// Clear the ACKFlag to wait for a status byte
					ES_Timer_InitTimer(RETRY_TIMER, RETRY_TIME);								// Reset the resend timer
				} else {																											// Otherwise
					TimerFlag = true;																						// Set the timer flag to indicate that there was a timeout without a pair acknowledgement
					printf("Set TMR Flag \r\n");																
				}
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == PAIR_FAIL_TIMER){ 	// If the 1s timer timed out
				CurrentState = Unpaired;																													// Move to the unpaired state
				ES_Timer_StopTimer(RETRY_TIMER);
				ES_Timer_StopTimer(PAIR_FAIL_TIMER);
				ES_Timer_StopTimer(UNPAIR_TIMER);
				printf("Entering the unpaired state from pair fail timer \r\n");
			}
			if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == UNPAIR_TIMER){			//// THIS IS TECHNICALLY NOT NEEDED IF WE USE THE SM's FROM THE CHARTS
				CurrentState = Unpaired;																													// Move to the unpaired state
				ES_Timer_StopTimer(RETRY_TIMER);
				ES_Timer_StopTimer(PAIR_FAIL_TIMER);																												////////////////////// Do not stop the pair fail timer since all timers timeout at the same time
				ES_Timer_StopTimer(UNPAIR_TIMER);
				printf("Entering the unpaired state from unpair timer \r\n");
			}
			if (ThisEvent.EventType == MANUAL_UNPAIR){
				CurrentState = Unpaired;																													// Move to the unpaired state
				ES_Timer_StopTimer(RETRY_TIMER);
				ES_Timer_StopTimer(PAIR_FAIL_TIMER);
				ES_Timer_StopTimer(UNPAIR_TIMER);
				printf("Entering the unpaired state from manual unpair\r\n");
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
	// TODO: Get DestinationAddress  																	////////////////////////////////////////////////////////////////////
	// TODO: Get Color																							///////////////////////////////////////////////////////////////////////
	PairData[1] = LobbyistNumber; 													// Save destination address						
	PairData[1] = (PairData[1]&BIT7LO)|Color;  							// Save color
	TransmitMessage(PairData, PAIR_ARRAY_SIZE, BROADCAST);	// Transmit message
}

/****************************************************************************
 Function
  TransmitControlPacket

 Description
	Sends a control packet to the Lobbyist

****************************************************************************/
static void TransmitControlPacket(void) { 
	// TODO: Encrypt
	// TODO: Update straight measure (byte 1) of control packet
	// TODO: Update turn measure (byte 2) of control packet
	// TODO: Update special action (byte3) of control packet
	
	ControlPacketArray[0] = CONTROL_HEADER;
//	ControlPacketArray[1] = 0x00;
//	ControlPacketArray[2] = 0x00;
//	ControlPacketArray[3] = 0x00;
//	ControlPacketArray[4] = 0x00;																													//////////////////////// MAKE THIS NEAT LATER
	
	ControlPacketArray[4] = ControlPacketArray[0]+ControlPacketArray[1]+ControlPacketArray[2]+ControlPacketArray[3];
	ControlPacket = &ControlPacketArray[0];
	uint8_t *ControlPointer = EncryptMessage(ControlPacket, CONTROL_ARRAY_SIZE);									////////// MAKE THIS NEAT LATER
//	for(int i = 0; i < CONTROL_ARRAY_SIZE; i++) {
//		printf("CTL Byte: %d \r\n", *ControlPointer);
//	}
	TransmitMessage(ControlPointer, CONTROL_ARRAY_SIZE, DestinationAddress); // Transmit message
}

/****************************************************************************
 Function
  TransmitEncryptionKey

 Description
	Sends the encryption key to the Lobbyist

****************************************************************************/
static void TransmitEncryptionKey(void) {
	uint8_t *Pointer = GenerateNewKey(); 
	for(int i = 1; i < ENCRYPTION_KEY_PACKET_SIZE; i++) {
		*(EncryptionKeyPacket+i) = *(Pointer+i-1);
		//printf("Encryption Key Byte: %d \r\n", *(EncryptionKeyPacket+i));
	}
	TransmitMessage(EncryptionKeyPacket, ENCRYPTION_KEY_PACKET_SIZE, DestinationAddress); // Transmit message;																			//////////////////////////////////////// TODO
}

/****************************************************************************
 Function
  InitPackets

 Description
	Initializes the arrays that will be used for sending pairing and control packets 

****************************************************************************/
static void InitPackets(void) {
	/*--------------------------- Intialize Data Arrays ------------------------------*/
	PairDataArray[0] = PAIR_REQ_HEADER;	
	PairDataArray[1] = 0x00;	

	ControlPacketArray[0] = CONTROL_HEADER;
	ControlPacketArray[1] = 0x00;
	ControlPacketArray[2] = 0x00;
	ControlPacketArray[3] = 0x00;
	ControlPacketArray[4] = 0x00;
	
	EncryptionKeyPacketArray[0] = ENCRYPTION_HEADER;
}

/***************************************************************************
 public functions
 ***************************************************************************/

/****************************************************************************
 Function
  SetStraight

 Description
	Sets the measure of the forward/backward command

****************************************************************************/
void SetStraight(int8_t StraightMeasure) {
	ControlPacketArray[1] = (uint8_t) StraightMeasure;
}

/****************************************************************************
 Function
  SetTurn

 Description
	Sets the measure of the left/right command 

****************************************************************************/
void SetTurn(int8_t TurnMeasure) {
	ControlPacketArray[2] = (uint8_t) TurnMeasure;
}

/****************************************************************************
 Function
  SetSpecialAction

 Description
	Sets the special action byte 

****************************************************************************/
void SetSpecialAction(uint8_t SpecialAction) {
	ControlPacketArray[3] = SpecialAction;
}


/****************************************************************************
 Function
  InitSensors

 Description
	Initializes the sensors

****************************************************************************/
static void InitSensors(void) {	
		
	// Initialize the pins associated with the motor as digital outputs
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R5;	// Initialize port F
		
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R5) != SYSCTL_PRGPIO_R5);	// Wait until initialized
	
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_3);	// Initialize F3 for IO
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) &= (BIT3LO);			// Initialize F3 for input
//printf("Initializing Sensors");
	uint8_t CurrentCapState =  HWREG(GPIO_PORTF_BASE + (GPIO_O_DATA + ALL_BITS)) & BIT3HI; // Read pin F3
//printf("Capacitive sensor measure: %d \r\n", CurrentCapState);
// Initialize PE0 as ADC for 1 dial
    ADC_MultiInit(1);
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


