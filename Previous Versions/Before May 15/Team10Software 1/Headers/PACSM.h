/****************************************************************************
 
  Header file for PACSM 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef PACSM_H
#define PACSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { Unpaired, Pairing, Paired } PACState_t ;

// Public Function Prototypes

bool InitPACSM ( uint8_t Priority );
bool PostPACSM( ES_Event ThisEvent );
ES_Event RunPACSM( ES_Event ThisEvent );
void ChangeStatus(uint32_t NewStatus);


#endif 

