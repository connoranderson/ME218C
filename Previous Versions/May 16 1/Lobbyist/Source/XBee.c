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

#include "UART.h"
#include "XBee.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)


// Identifiers for useful API packets
#define TX_API 0x01 // Sending a packet
#define TX_RESULT_API 0x89 // Result after sending a packet
#define RX_API 0x81 // Receive packet
#define DATA_BYTES 0x08 // Location of data bytes in incoming message
#define INCOMING_MESSAGE_OVERHEAD 9 // Number of bytes besides data bytes in incoming message
#define ADDRESS_MSB 4 // Location of address of sender
#define ADDRESS_LSB 5
#define API_IDENTIFIER 3 // Location of API Identifier
#define TRANSMIT_RESULT_STATUS 5 // Location of status byte for a transmit result package
#define STATUS_SUCCESS 0
#define STATUS_NO_ACK 1
#define STATUS_CCA_FAILURE 2
#define TX_API_IDENTIFIER 0x01
#define TRANSMIT_OVERHEAD 0x09 // Number of other bytes (besides data) involved in transmission

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static bool AssemblePacket(uint8_t *transmitData, uint8_t SizeOfData, uint16_t destAddress);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t outgoingMessageArray[32];
static uint8_t *outgoingMessage = &outgoingMessageArray[0];
static uint8_t frameID = 1; // Keeps track of which message we're sending



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitXbee
		 
 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
bool InitXBee (void)
{
// Init UART 1
	InitUart1();
	
	return true;
}


/****************************************************************************
 Function
  AssemblePacket
 Description
	Creates a packet for XBee using data from parameter.

****************************************************************************/
static bool AssemblePacket(uint8_t *transmitData, uint8_t SizeOfData, uint16_t destAddress) {
	
	*outgoingMessage = 0x7E;
	*(outgoingMessage+1) = 0x00;
	*(outgoingMessage+2) = SizeOfData+5;
	*(outgoingMessage+3) = TX_API_IDENTIFIER;
	*(outgoingMessage+4) = frameID;
	*(outgoingMessage+5) = destAddress>>8;
	*(outgoingMessage+6) = destAddress;
	*(outgoingMessage+7) = 0x00;
	
	for(int i = 0; i < SizeOfData; i++){
		*(outgoingMessage+8+i) = *(transmitData + i);		
	}
	
	uint8_t checkSum = ComputeChecksum(outgoingMessage, SizeOfData + 8);
	//printf("\r\nChecksum = %d", checkSum);
	*(outgoingMessage+SizeOfData+8) = checkSum;
	
	return true;
}	

/****************************************************************************
 Function
  TransmitMessage

 Description
	Sends a message interfacing with the UART

****************************************************************************/
void TransmitMessage(uint8_t *transmitData, uint8_t SizeOfData, uint16_t destAddress){ 
	AssemblePacket(transmitData, SizeOfData, destAddress);
	LoadData(outgoingMessage,SizeOfData + TRANSMIT_OVERHEAD);
	BeginMessageTransmission();
}


/****************************************************************************
 Function
  ComputeChecksum

 Description
	Computes checksum of given Array. Sums from the frame data (starting with byte 4
	through the end of the frame). Goes to size - 1 to ignore the CheckSum (should
  be 0 anyways).

****************************************************************************/
uint8_t ComputeChecksum(uint8_t *inputArray, uint8_t sizeOfArray){ 
		uint8_t checkSum = 0;
		for(int i=3; i < sizeOfArray; i++){
			checkSum += *(inputArray + i);
		}
		return 0xFF - checkSum;
}

/****************************************************************************
 Function
  GetMessageData

 Description
	Returns a pointer to the data contained in the most recent message received

****************************************************************************/

uint8_t * GetMessageData( void){
	uint8_t *firstIndex = GetMessage();
	return (firstIndex + DATA_BYTES);	
}

/****************************************************************************
 Function
  GetMessageDataSize

 Description
	Returns the size of the data most recently received

****************************************************************************/

uint8_t GetMessageDataSize( void){
	return GetMessageSize() - INCOMING_MESSAGE_OVERHEAD;	
}

/****************************************************************************
 Function
  GetSenderAddress

 Description
	Returns the sender address from the last received message

****************************************************************************/

uint16_t GetSenderAddress( void){
	uint8_t *firstIndex = GetMessage();
	uint16_t senderAddress = *(firstIndex + ADDRESS_MSB);
	senderAddress = senderAddress << 8; // Shift msb over to make room for lsb of sender
	senderAddress |= *(firstIndex + ADDRESS_LSB);
	return senderAddress;	
}

/****************************************************************************
 Function
  GetApiIdentifier

 Description
	Returns the API identifier from the last received message

****************************************************************************/

uint8_t GetApiIdentifier( void){
	uint8_t *firstIndex = GetMessage();
	uint8_t apiIdentifier = *(firstIndex + API_IDENTIFIER);
	return apiIdentifier;	
}

/****************************************************************************
 Function
  CheckForErrors

 Description
	Detects any errors in the incoming message and returns an int 0-4 to 
	indicate which error, if any, existed in the message.

0: tx successful
1: rx successful
2: checksum error
3: no ack failure
4: cca failure

****************************************************************************/

uint8_t CheckForErrors(uint8_t *inputMessage, uint8_t sizeOfMessage){
	
	// See if we had a checksum error
	uint8_t checkSum = ComputeChecksum(inputMessage,sizeOfMessage-1); // Pass packet (w/o checksum) to compute checksum
	//printf("Checksum : %d", checkSum);
	if(checkSum != *(inputMessage + sizeOfMessage - 1)){ // -1 acconts for checksum being at index size-1 (array starts at 0 index)
		return CHECKSUM_FAILURE;		
	}
	
	// See if we're looking at a transmit result message
	if(*(inputMessage + API_IDENTIFIER) == TX_RESULT_API){
		if(*(inputMessage + TRANSMIT_RESULT_STATUS) == STATUS_SUCCESS){
			return TX_SUCCESS;
		}else if(*(inputMessage + TRANSMIT_RESULT_STATUS) == STATUS_NO_ACK){
			return NO_ACK_FAILURE;
		}else if(*(inputMessage + TRANSMIT_RESULT_STATUS) == STATUS_CCA_FAILURE){
			return CCA_FAILURE;
		}
	}
	// Return a success if a receive packet made it this far
	return RX_SUCCESS;
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

