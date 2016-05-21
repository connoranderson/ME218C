/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef StepService_H
#define StepService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { WaitingForNextStep } StepState_t ;

// Public Function Prototypes

bool InitStepService ( uint8_t Priority );
bool PostStepService( ES_Event ThisEvent );
ES_Event RunStepService( ES_Event ThisEvent );
uint8_t getCurrentDriveMode();


#endif 

