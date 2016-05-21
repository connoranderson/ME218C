/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef UART_Service_H
#define UART_Service_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { Debouncing, WaitingForPress } ButtonState_t ;

// Public Function Prototypes

bool InitUartService ( uint8_t Priority );
bool PostUartService( ES_Event ThisEvent );
ES_Event RunUartService( ES_Event ThisEvent );


#endif /* LCDService_H */

