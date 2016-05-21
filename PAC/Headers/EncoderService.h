/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef EncoderService_H
#define EncoderService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { Finding_Period } EncoderState_t ;

// Public Function Prototypes

bool InitEncoderService ( uint8_t Priority );
bool PostEncoderService( ES_Event ThisEvent );
ES_Event RunEncoderService( ES_Event ThisEvent );



#endif 

