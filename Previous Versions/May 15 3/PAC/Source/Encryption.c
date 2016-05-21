#define ALL_BITS (0xff<<2)
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

#include <time.h>
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
#define KEY_LENGTH 32


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t encryptionKeyArr[32]; // Allocate space for full encryption key
static uint8_t *encryptionKey = &encryptionKeyArr[0];
static uint8_t encryptedMessageArr[32]; // Allocate space for full encrypted message
static uint8_t *encryptedMessage = &encryptedMessageArr[0];
static uint8_t decryptedMessageArr[32]; // Allocate space for full encrypted message
static uint8_t *decryptedMessage = &decryptedMessageArr[0];
static uint8_t currentCipherPosition = 0; // Keeps track of which cipher position we're currently at

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     generateNewKey

 Description
     Randomly generates a new, 32 byte key

****************************************************************************/
uint8_t * GenerateNewKey( void){
	currentCipherPosition = 0;
	uint16_t TimeSeed;
	TimeSeed = ES_Timer_GetTime();
	srand(TimeSeed);
	for(int i = 0; i < KEY_LENGTH; i ++){
		uint8_t nextByte = rand();
		*(encryptionKey + i) = nextByte;
	}
	return encryptionKey;
}

/****************************************************************************
 Function
     GetKey

 Description
     Returns the current Key

****************************************************************************/
uint8_t * GetKey( void){
	return encryptionKey;
}

/****************************************************************************
 Function
     SetKey

 Description
     Sets the key to a predetermined value (useful for debugging)

****************************************************************************/
void SetKey(uint8_t *newKey){  
	currentCipherPosition = 0;
	for(int i = 0; i < KEY_LENGTH; i ++){
		*(encryptionKey + i) = *(newKey + i);
	}
}

/****************************************************************************
 Function
     EncryptMessage

 Description
     Incrypts a message using the current 32 byte key

****************************************************************************/
uint8_t * EncryptMessage(uint8_t *inputMessage, uint8_t inputMessageLength){
	for(int i = 0; i < inputMessageLength; i++){
		*(encryptedMessage + i) = *(inputMessage+i)^*(encryptionKey + currentCipherPosition);
	}
	return encryptedMessage;
}

/****************************************************************************
 Function
     DecryptMessage

 Description
     Used by Lobbyist to decrypt a message from the PAC

****************************************************************************/
uint8_t * DecryptMessage(uint8_t *inputMessage, uint8_t inputMessageLength){
	for(int i = 0; i < inputMessageLength; i++){
		*(decryptedMessage + i) = *(inputMessage+i)^*(encryptionKey + currentCipherPosition);
	}
	return decryptedMessage;
}

/****************************************************************************
 Function
     RotateCipher

 Description
     Rotates cipher by one byte for next message

****************************************************************************/
void RotateCipher(void){
	currentCipherPosition++;
	if (currentCipherPosition == 32) {
		currentCipherPosition = 0;
	}
	//printf("CurrentCipherPosition: %d \r\n", currentCipherPosition);
}


