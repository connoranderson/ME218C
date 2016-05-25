#define ALL_BITS (0xff<<2)
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"

#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "LobbyistNumber.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLobbyistNumber

 Description
     Initializes the pin associated with setting the desired lobbyist number
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
bool InitLobbyistNumber (void)
{
   // Initialize Port D pins 0-3
    // Set bit 3 and enable Port D
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3);
    // Set Port D bits 0-3 to be a digital I/O pin
    HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    // Make bits 0-3 on Port D to be inputs
    HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= ~(GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	
	return true;
}

/****************************************************************************
 Function
     InitLobbyistNumber

 Description
     Initializes the pin associated with setting the lobbyist number
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
uint8_t GetLobbyistNumber(void)
{
  uint8_t LobbyistNumberReading = HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) & (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	uint8_t LobbyistNumber = 0x00;
	if((LobbyistNumberReading & BIT0HI) != 0x00) {
		LobbyistNumber = 0x00;
	} else if((LobbyistNumberReading & BIT1HI) != 0x00) {
		LobbyistNumber = 0x01;
	} else if((LobbyistNumberReading & BIT2HI) != 0x00) {
		LobbyistNumber = 0x02;
	} else if((LobbyistNumberReading & BIT3HI) != 0x00) {
		LobbyistNumber = 0x03;
	}
	printf("Lobbyist number reading: %d \r\n", LobbyistNumberReading);
	printf("Lobbyist number: %d \r\n", LobbyistNumber);
	return LobbyistNumber;
}



/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

