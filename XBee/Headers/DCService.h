/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef DCService_H
#define DCService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { Driving } DCState_t ;

// Public Function Prototypes

bool InitDCService ( uint8_t Priority );
bool PostDCService( ES_Event ThisEvent );
ES_Event RunDCService( ES_Event ThisEvent );



#endif 

