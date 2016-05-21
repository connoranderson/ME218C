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

#include "ReadMyo.h"

/*----------------------------- Module Defines ----------------------------*/
#define REST 0x00
#define FORWARD 0x01
#define BACKWARD 0x02
#define SPEED 127
#define MAX_RAW_ROLL 16// NEED TO SET THIS
#define ROLL_BITS 0x3C
#define GESTURE_BITS 0xC0


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/


/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
  InitMyoInputs
 Description
	Initializes the pins associated with reading inputs from the MyoBand through the PI

****************************************************************************/

bool InitMyoInputs(void) {
	// Set up the pins associated with reading inputs from the MyoBand through the PI

	// Initialize Port A pins 2-7
    // Set bit 1 and enable Port A
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0);
    // Set Port A bits 2-7 to be a digital I/O pins
    HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    // Make bits 2-7 on Port A to be inputs
    HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) &= ~(GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
  return true;
}




/****************************************************************************
 Function
  GetGesture
 Description
	Returns the uint8_t representation of the gesture recognized from the MyoBand

****************************************************************************/

int8_t GetGesture(void) {
  uint8_t Gesture = 0x00;             // Initialize variable uint8_t Gesture to 0x00
  // Read the necesary pins for reading the gesture and save them into the Gesture byte
	Gesture = HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) & GESTURE_BITS;
  int8_t StraightMeasure = 0x00;      // Initialize variable StraightMeasure to 0x00
  // Depending on the gesture, set the straight measure
  switch(Gesture) {                   
    case REST:                        // If the Pi is signaling to rest  
      StraightMeasure = 0;            // Set the straight measure to 0
    break;                            
    case FORWARD:                     // If the Pi is signaling forward
      StraightMeasure = SPEED;        // Set the straight measure to full speed forward
    break;
    case BACKWARD:                    // If the Pi is signaling backward
      StraightMeasure = -1*SPEED;     // Set the straight measure to full speed backward
    break;
    default:
    break;
  }
  return StraightMeasure;
}


/****************************************************************************
 Function
  GetRoll

 Description
	Sends a message interfacing with the UART

****************************************************************************/

int8_t GetRoll(void) {
  uint8_t RawRoll = 0x00;                           // Initialize variable uint8_t RawRoll to 0x00
  // Read the necesary pins for reading the roll and save them as the first four bits in a byte
	RawRoll = HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) & ROLL_BITS;
  int8_t SplitRole = RawRoll-MAX_RAW_ROLL/2;              // Initialize split role to make the conversion easier so that the role is split evenly bewteen positive and negative values
  int8_t TurnMeasure = SPEED*SplitRole/(RawRoll/2); // Initialize variable int8_t TurnMeasure and set it based on Roll
  return TurnMeasure;
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

