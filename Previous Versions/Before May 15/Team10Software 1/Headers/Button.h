/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Button_H
#define Button_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { Debouncing, WaitingForPress } ButtonState_t ;

// Public Function Prototypes

bool InitButtonService ( uint8_t Priority );
bool PostButtonService( ES_Event ThisEvent );
ES_Event RunButtonService( ES_Event ThisEvent );


#endif /* LCDService_H */

