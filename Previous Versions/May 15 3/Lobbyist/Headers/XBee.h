/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef XBee_H
#define XBee_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// Public Function Prototypes

bool InitXBee(void);
void TransmitMessage(uint8_t *transmitData, uint8_t SizeOfData, uint16_t destAddress);
uint8_t ComputeChecksum(uint8_t *inputArray, uint8_t sizeOfArray);
uint8_t * GetMessageData( void);
uint8_t GetMessageDataSize( void);
uint16_t GetSenderAddress( void);
uint8_t CheckForErrors(uint8_t *inputMessage, uint8_t sizeOfMessage);
uint8_t GetApiIdentifier( void);

enum rx_status{
	TX_SUCCESS = 0,
	RX_SUCCESS,
	CHECKSUM_FAILURE,
	NO_ACK_FAILURE,
	CCA_FAILURE,	
};

#endif
