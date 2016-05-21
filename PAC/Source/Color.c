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

#include "Color.h"

/*----------------------------- Module Defines ----------------------------*/
#define RED 0x00
#define BLUE 0x80

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitColor

 Description
     Initializes the pin associated with setting the color
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
bool InitColor (void)
{
   // Initialize Port D pin 6
    // Set bit 3 and enable Port D
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3);
    // Set Port D bit 6 to be a digital I/O pin
    HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= GPIO_PIN_6;
    // Make bit 6 on Port D to be inputs
    HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= ~GPIO_PIN_6;
	
	return true;
}

/****************************************************************************
 Function
     InitColor

 Description
     Initializes the pin associated with setting the color
		 
 Notes

 Author
    Connor Anderson and Will Roderick, 05/05/2016
****************************************************************************/
uint8_t GetColor(void)
{
//	puts("In get color");
  uint8_t ColorReading = HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) & BIT6HI;
//	puts("Read pin");
	uint8_t Color;
	if(ColorReading > 0x00) {
	Color = BLUE;
	} else {
	Color = RED;
	}
	return Color;
}



/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

