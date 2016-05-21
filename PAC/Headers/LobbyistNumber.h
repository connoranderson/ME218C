/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef LobbyistNumber_H
#define LobbyistNumber_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// Public Function Prototypes

bool InitLobbyistNumber(void);
uint8_t GetLobbyistNumber(void);

#endif
