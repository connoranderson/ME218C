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

#include "Badge.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitBadge

 Description
     Initializes the pins for the badge
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
bool InitBadge (void)
{
	 	// Initialize Port E pins 0 and 1
    // Set bit 1 and enable Port E
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4);
    // Set Port E bits 0 and 1 to be a digital I/O pins
    HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= GPIO_PIN_0 | GPIO_PIN_1;
    // Make bits 0 and 1 on Port E to be an output
    HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) &= ~(GPIO_PIN_0 | GPIO_PIN_1);
	
	return true;
}

/****************************************************************************
 Function
	ReadBadge

 Description
	Reads the badge and updates the value

****************************************************************************/
uint8_t ReadBadge(void) { 
	uint8_t LobbyistNumber = HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS)) & (BIT0HI | BIT1HI);
	return LobbyistNumber;
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

