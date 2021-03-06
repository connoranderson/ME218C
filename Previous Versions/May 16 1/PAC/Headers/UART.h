/****************************************************************************
 
  Header file for Test Harness Service0 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef UART_H
#define UART_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// Public Function Prototypes

void InitUart1 ( void);
void UART1_ISR( void);
void LoadData(uint8_t *transmitData, uint8_t SizeOfArray);
bool BeginMessageTransmission( void);
bool SendMessage(uint8_t data);
bool SendTestMessage(void);
uint8_t GetMessageSize( void);
uint8_t * GetMessage( void);



#endif /* UART_H */

