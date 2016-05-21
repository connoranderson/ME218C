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

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static bool AssemblePacket(uint8_t *transmitData, uint8_t SizeOfData, uint16_t destAddress);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t outgoingMessageArray[32];
static uint8_t *outgoingMessage = &outgoingMessageArray[0];
static uint8_t frameID = 0; // Keeps track of which message we're sending



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
////	SendTestMessage();
////	uint8_t firstTransmit[20] = {0x7E, 0x00, 0x10, 0x01, 0xAA,0xFF,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0};
////	uint8_t *p = &firstTransmit[0]; 
////	uint8_t checkSum = ComputeChecksum(p, 20);
////	firstTransmit[19] = checkSum;
//	printf("checkSum = %d", checkSum);
//	LoadData(p, SizeOfArray);
	
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
	*(outgoingMessage+3) = 0x01;
	*(outgoingMessage+4) = frameID;
	*(outgoingMessage+5) = destAddress>>8;
	*(outgoingMessage+6) = destAddress;
	*(outgoingMessage+7) = 0x00;
	
	for(int i = 0; i < SizeOfData; i++){
		*(outgoingMessage+8+i) = *(transmitData + i);		
	}
	
	uint8_t checkSum = ComputeChecksum(outgoingMessage, SizeOfData + 8);
	*(outgoingMessage+SizeOfData+8) = checkSum;
	
	return true;
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
  SetCommand

 Description
	Computes checksum of given Array. Sums from the frame data (starting with byte 4
	through the end of the frame). Goes to size - 1 to ignore the CheckSum (should
  be 0 anyways).

****************************************************************************/
uint8_t SetCommand(uint8_t *inputArray, uint8_t sizeOfArray){ 
		uint8_t checkSum = 0;
		for(int i=3; i < sizeOfArray; i++){
			checkSum += *(inputArray + i);
		}
		return 0xFF - checkSum;
}



/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

