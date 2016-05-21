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
uint8_t ComputeChecksum(uint8_t *inputArray, uint8_t sizeOfArray);
#endif
