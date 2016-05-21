/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Encryption_H
#define Encryption_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// Public Function Prototypes

uint8_t * GenerateNewKey( void);
void SetKey(uint8_t *newKey);
uint8_t * DecryptMessage(uint8_t *inputMessage, uint8_t inputMessageLength);
void RotateCipher(void);
uint8_t * GetKey( void);

#endif
