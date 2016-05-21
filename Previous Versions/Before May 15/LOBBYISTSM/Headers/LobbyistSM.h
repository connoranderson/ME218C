/****************************************************************************
 Header file for the Lobbyist state machine.

 ****************************************************************************/

#ifndef LobbyistSM_H
#define LobbyistSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// State definitions for use with the query function
typedef enum {InitPState, Unpaired, Pairing, Paired} LobbyistState_t ;

// Public Function Prototypes

ES_Event RunLobbyistSM( ES_Event CurrentEvent );
bool PostLobbyistSM( ES_Event ThisEvent );
bool InitLobbyistSM ( uint8_t Priority );
LobbyistState_t QueryLobbyistSM(void);

#endif /*LobbyistSM_H */

