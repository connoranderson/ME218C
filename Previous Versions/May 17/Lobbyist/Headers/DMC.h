/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef DMC_H
#define DMC_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// Public Function Prototypes
bool InitDMC(void);
bool SetColor(uint8_t Color);
ES_Event RunDMC( ES_Event CurrentEvent );
bool PostDMC( ES_Event ThisEvent );
bool InitDMC( uint8_t Priority );

#endif
