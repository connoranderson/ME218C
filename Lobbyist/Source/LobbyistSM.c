/****************************************************************************
 Module
   LobbyistSM.c

 Description
   This is the high level state machine for the Lobbyist.

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
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

#define DMC_UNPAIRED 0x00
#define DMC_PAIRED 0x01

#define BRK BIT0HI
#define MAN_UNPR BIT1HI
#define SP_ACT BIT2HI

#define BLUE 0x01
#define RED 0x00


/*---------------------------- Module Functions ---------------------------*/
static void InitPackets(void);
static void TransmitStatusPacket(uint8_t PairedStatus);
static bool CheckLobbyistNumber(void);
static void ExecuteControl(void);
static uint8_t GetCheckSum(uint8_t *incomingData);
static bool UpdateDMC(uint8_t DMCPairing, uint8_t DMCColor);

/*---------------------------- Module Variables ---------------------------*/
static LobbyistState_t CurrentState;
static uint8_t MyPriority;
static uint8_t PairTimeLeft;
static uint8_t StatusPacketArray[STATUS_PACKET_SIZE];
static uint8_t *StatusPacket = &StatusPacketArray[0];
static uint8_t incomingmessagearr[8];
static uint8_t *incomingMessage = &incomingmessagearr[0];
static uint8_t decryptedMessageArr[8];
static uint8_t *decryptedMessage = &decryptedMessageArr[0];
static uint8_t LobbyistNumber = 0x0D; // Our ID which the PAC uses to pair with us
                                      
static uint16_t pairedAddress; // Address of the PAC we are paired to

static uint8_t command = 0;
static uint8_t *lastCommandFrame1 = &command;
//static uint8_t *lastCommandFrame2 = &command;
//static uint8_t *lastCommandFrame3 = &command;
static uint8_t encryptedChecksum;

static uint8_t Color;

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
						printf("Init Steering \r\n");
						InitSteering();					// Initialize the steering systems
						printf("Init Packets \r\n");					
						InitPackets(); 					// Initialize packet array
						printf("Init XBee \r\n");
            InitXBee(); 						// Initialize the comms
						printf("Init Lift Fan \r\n");
						InitLiftFan();					// Initialize the lift fan
            printf("Init Badge \r\n");
						InitBadge();						//Initialize the Badge
            CurrentState = WaitingForPair;
        }
    break;

    case WaitingForPair :
      switch ( ThisEvent.EventType )
      {
        //If the event is a pair request, change state to pairing and wait for the key.
        case MESSAGE_RECEIVED:
					// Read incoming message
					incomingMessage = GetMessageData();
					// Check that message is a pair request and that it's meant for us
					if(*incomingMessage == PAIR_REQUEST_HEADER && CheckLobbyistNumber() && GetApiIdentifier() == INCOMING_PACKET_API_IDENTIFIER){  // Ensure that this is not a Tx success receive
						printf("\r\n Current Message Size: %d ", GetMessageDataSize());
						// If we passed the checks, we're now paired. Start timers.
						printf("\n\r\nPair request received in unpaired state.");
						ES_Timer_InitTimer(UNPAIR_TIMER, UNPAIR_TIME); //Start the 45s timer
						ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC); //Start the 1s timer
						
						
						TurnOnLiftFan();     					//Start the lift fan
						// Update the DMC
							Color = *(incomingMessage + 1);
							Color &= BIT7HI;
							if(Color > 0x00) {
								Color = BLUE;
							} else {
								Color = RED;
							}
							UpdateDMC(DMC_PAIRED,Color); 	//Update DMC
						 // ES_Timer_InitTimer(DMC_TIMER, ONE_SEC); //Start the 1s DMC timer
						// Recognize our new pac daddy
						pairedAddress = GetSenderAddress() ;
						// Send an ACK back to the sender
						TransmitStatusPacket(PAIRED); 
						// Set the pair time
						PairTimeLeft = 45;
						CurrentState = PairedAwaitEncryptionKey;
				}
        break;
//				case ES_TIMEOUT:
//					if(ThisEvent.EventParam == DMC_TIMER) {
//							UpdateDMC(DMC_UNPAIRED,Color); 	//Update DMC
//					}	
				break;
        default :
            ; 
      }
    break;

    case PairedAwaitEncryptionKey:
      switch(ThisEvent.EventType){
        case MESSAGE_RECEIVED:
					// Check sender and message
					incomingMessage = GetMessageData();
					uint16_t sender = GetSenderAddress() ;
					if(sender == pairedAddress && *incomingMessage == KEY_SEND_HEADER){
						printf("\n\r\nKey received in pairing state.");
						//Save the key 
						SetKey((incomingMessage+1)); // Make sure to +1 to start after header byte
						
						ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC); //Restart the 1s timer
						TransmitStatusPacket(PAIRED); //transmit status with the pairing bit set.
						CurrentState = PairedAwaitControl;
						printf("\r\n In paired await control");
				}
        break;
        case ES_TIMEOUT: // if there is a timeout on the 1s timer
					printf("\n\r\n1s timeout in pairing - moving to awaiting pair.");
					TurnOffLiftFan(); 										//Turn off the lift fan
					//Update DMC to avaiable for pairing
					ES_Timer_StopTimer(UNPAIR_TIMER); 		//Disable the 45s pairing timer
					ES_Timer_StopTimer(PAIR_FAIL_TIMER); 	//Disable the 1s transmit timeout timer
					TransmitStatusPacket(UNPAIRED); 			//Transmit status with the pairing bit cleared.
					CurrentState = WaitingForPair;
        break;
      }
    break;

    case PairedAwaitControl:
      switch(ThisEvent.EventType){
        case MESSAGE_RECEIVED:
					// See who sent the message
					uint16_t sender = GetSenderAddress() ;
					if(sender == pairedAddress){  // Only attempt decrypt if from sender
						// Check if unencrypted message had the same CommandFrame as the last one
						uint8_t  *thisCommandFrame1 = GetMessageData();
						//uint8_t  *thisCommandFrame2 = thisCommandFrame1+;
						//uint8_t  *thisCommandFrame3 = thisCommandFrame2+1;
						GetCheckSum(thisCommandFrame1);
						// IF not the same, decrypt and send ack
						if(*(thisCommandFrame1) != *(lastCommandFrame1) || *(thisCommandFrame1 + 1) != *(lastCommandFrame1 + 1)){// || *(thisCommandFrame2) != *(lastCommandFrame2) || *(thisCommandFrame3) != *(lastCommandFrame3)){
							printf("\r\n Decrypting Message");
							decryptedMessage = DecryptMessage(GetMessageData(),GetMessageDataSize());
							//RotateCipher();
							*lastCommandFrame1 = *thisCommandFrame1;
							//*lastCommandFrame2 = *thisCommandFrame2;
							//*lastCommandFrame3 = *thisCommandFrame3;
						}
						// If we received the same message again, resend the ACK					

						if(*decryptedMessage == CNTRL_REQUEST_HEADER){
							//printf("\n\r\nCommand received in the paired state.");
							ES_Timer_InitTimer(PAIR_FAIL_TIMER, ONE_SEC); //Restart the control command timer.
							ExecuteControl(); //Execute the control command.
							TransmitStatusPacket(PAIRED);// ACK YAY. Transmit the status with pairing bit set.
						} else {
							printf("\n\r Not a valid control packet received, Header: %d", *incomingMessage);
						}
				}
        break;

        case ES_TIMEOUT: //Either the 1s or 45s timer
				case MANUAL_UNPAIR: // Or if the pair is manual
				case DECRYPT_FAIL:  // Or if the decryption failed
//					if(ThisEvent.EventParam == DMC_TIMER) {
//						printf("DMC Timeout");
//							UpdateDMC(DMC_PAIRED,Color); 	//Update DMC
//						  //ES_Timer_InitTimer(DMC_TIMER, ONE_SEC); //Start the 1s DMC timer
//					} else 

					if(ThisEvent.EventParam == UNPAIR_TIMER){
						// Keep track of how long is left on UNPAIR_TIMER
              PairTimeLeft--;
//              printf("\n\r\nTime until unpair: %ds", PairTimeLeft);
						// If timer has run for 45 s
              if(PairTimeLeft == 0){    
                CurrentState = WaitingForPair;
								TurnOffLiftFan(); 										//Turn off the lift fan
								UpdateSteering(OFF,OFF);							//Turn off the propulsion fans
								//Update DMC to avaiable for pairing
								ES_Timer_StopTimer(UNPAIR_TIMER); 		//Disable the 45s pairing timer
								ES_Timer_StopTimer(PAIR_FAIL_TIMER); 	//Disable the 1s transmit timeout timer
                printf("\n\r\nUnpairing.");
						// If timer is not at 0, reinitialize the timer again for 1s to keep counting
              }else{
								 ES_Timer_InitTimer(UNPAIR_TIMER, UNPAIR_TIME); //Start the 45s timer
							}								
						} else if (ThisEvent.EventParam == PAIR_FAIL_TIMER || ThisEvent.EventType == DECRYPT_FAIL || ThisEvent.EventType == MANUAL_UNPAIR) {  
							// If we have an event NOT on the 45s timer, unpair
								printf("\n\r\nTimeout on 1s the paired state, manual unpair, or decrypt failure. Moving to waiting for pair \r\n");
								TurnOffLiftFan(); 										//Turn off the lift fan
								UpdateSteering(OFF,OFF);							//Turn off the propulsion fans
								//Update DMC to avaiable for pairing
								ES_Timer_StopTimer(UNPAIR_TIMER); 		//Disable the 45s pairing timer
								ES_Timer_StopTimer(PAIR_FAIL_TIMER); 	//Disable the 1s transmit timeout timer
								if(ThisEvent.EventType == DECRYPT_FAIL){	// If this event came from a decryption failure 		//// WEHERE DOES THIS COME FROM?
									TransmitStatusPacket(DECRYPT_FAILURE);	// Transmit status with the decryption fail bit set
								} else {
									TransmitStatusPacket(UNPAIRED); 				//Transmit status with the pairing bit cleared.
								}
								CurrentState = WaitingForPair;
								UpdateDMC(DMC_UNPAIRED,Color); 	//Update DMC
              } else if (ThisEvent.EventType == ES_TIMEOUT) {
                 printf("Timer failure on timer: %d", ThisEvent.EventParam);
							} else {
								 printf("Unknown failure");
								 printf("EventType: %d", ThisEvent.EventType);
							}
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


/****************************************************************************
 Function
  TransmitStatusPacket

 Description
	Sends a status packet to the Lobbyist

****************************************************************************/
static void TransmitStatusPacket(uint8_t PairedStatus) { 
	StatusPacketArray[1] = PairedStatus;
	StatusPacketArray[2] = encryptedChecksum;
	TransmitMessage(StatusPacket, STATUS_PACKET_SIZE, pairedAddress);
}

/****************************************************************************
 Function
  InitPackets

 Description
	Initializes the arrays that will be used for sending status packets 

****************************************************************************/
static void InitPackets(void) {
	/*--------------------------- Intialize Data Arrays ------------------------------*/
	StatusPacketArray[0] = 0x03;	
}


/****************************************************************************
 Function
  GetLobbyistNumber

 Description
	Looks inside the received data (a pair byte from PAC) to figure out whether
	we are the desired Lobbyist to pair with.

****************************************************************************/
static bool CheckLobbyistNumber(void) {
	uint8_t pairData = *(incomingMessage + 1);
	pairData &= BIT7LO; //(BIT1HI | BIT0HI);
	LobbyistNumber =  ReadBadge(); // Update the lobbyist number based on the badge.
	if(pairData == LobbyistNumber){
		return true;
	}
	return false;
}

/****************************************************************************
 Function
  GetCheckSum

 Description
	Looks inside the received data to see what value is in the CheckSum byte.
	Used for returning an encrypted checksum to be returned in a status packet,
	per the new, updated Comm Protocol.

****************************************************************************/
static uint8_t GetCheckSum(uint8_t *incomingData) {
	encryptedChecksum = *(incomingData + CNTRL_CHECKSUM);
	return encryptedChecksum;
}


/****************************************************************************
 Function
  ExecuteControl

 Description
	Executes the required commands in the control packet

****************************************************************************/
static void ExecuteControl(void) {
	int8_t FWD_BCK = (int8_t) *(decryptedMessage+1);
//	 printf("\r\n FWD/BCKWD: %d ", FWD_BCK);
	int8_t LFT_RGHT = (int8_t) *(decryptedMessage+2);
//	 printf("\r\n LFT/RGHT: %d ",LFT_RGHT);
//	 printf("\r\n Special Action: %d ", *(decryptedMessage+3));
	UpdateSteering(FWD_BCK,LFT_RGHT);
	
	int8_t CTL3 = *(decryptedMessage+3);
	
	if((CTL3 & MAN_UNPR) != 0) {
		ES_Event ManualUnpairEvent;
		ManualUnpairEvent.EventType = MANUAL_UNPAIR;
		 PostLobbyistSM(ManualUnpairEvent);
		printf("Manual unpair detected \r\n");
	}
	if((CTL3 & SP_ACT) != 0) {
		printf("Special action detected \r\n");
	}
	if((CTL3 & BRK) != 0) {
		printf("Brake detected \r\n");
	}
}



/****************************************************************************
 Function
  UpdateDMC

 Description
	Updates the DMC

****************************************************************************/
static bool UpdateDMC(uint8_t DMCPairing, uint8_t DMCColor) {
	ES_Event DMCPairUpdateEvent;
	DMCPairUpdateEvent.EventType = UPDATE_DMC_PAIRING;
	DMCPairUpdateEvent.EventParam = DMCPairing;
	PostDMC(DMCPairUpdateEvent);
	ES_Event DMCColorUpdateEvent;
	DMCColorUpdateEvent.EventType = UPDATE_DMC_COLOR;
	DMCColorUpdateEvent.EventParam = DMCColor;
	PostDMC(DMCColorUpdateEvent);
	ES_Event DMCUpdateEvent;
	DMCUpdateEvent.EventType = UPDATE_DMC;
	PostDMC(DMCUpdateEvent);
	printf("Updated DMC \r\n");
	return true;
}
