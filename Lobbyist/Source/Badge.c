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
#define BADGE_1 100
#define BADGE_2 200
#define BADGE_3 300
#define BADGE_4 400
#define LOBBYIST_NUMBER1 1
#define LOBBYIST_NUMBER2 2
#define LOBBYIST_NUMBER3 3
#define LOBBYIST_NUMBER4 4
#define TOLERANCE 50


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

static uint8_t ToBadgeNumber(uint16_t badgeAnalogValue)

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
// Initialize PE0 and PE1 as ADC for 2 analog inputs
    ADC_MultiInit(2);
	
	return true;
}

/****************************************************************************
 Function
	ReadBadge

 Description
	Reads the badge and updates the value

****************************************************************************/
uint8_t ReadBadge(void) { 
    ADC_MultiRead(ADResults);
    uint16_t currentBadgeValue = ADResults[0];
    printf("\r\n BadgeAnalogValue: %d", currentBadgeValue);
	uint8_t badgeNumber = ToBadgeNumber(currentBadgeValue);
	return badgeNumber;
}

/****************************************************************************
 Function
    ToBadgeNumber

 Description
    Converts an analog badge value to a lobbyist number

****************************************************************************/
static uint8_t ToBadgeNumber(uint16_t badgeAnalogValue){
    if(badgeAnalogValue < (BADGE_1 + TOLERANCE) && badgeAnalogValue > (BADGE_1 - TOLERANCE)){
        return LOBBYIST_NUMBER1;
    }else if(badgeAnalogValue < (BADGE_2 + TOLERANCE) && badgeAnalogValue > (BADGE_2 - TOLERANCE)){
        return LOBBYIST_NUMBER2;
    }else if(badgeAnalogValue < (BADGE_3 + TOLERANCE) && badgeAnalogValue > (BADGE_3 - TOLERANCE)){
        return LOBBYIST_NUMBER3;
    }else if(badgeAnalogValue < (BADGE_4 + TOLERANCE) && badgeAnalogValue > (BADGE_4 - TOLERANCE)){
        return LOBBYIST_NUMBER4;
    }else{
        printf("Badge not recognized. Badge no: %d", badgeAnalogValue);
    }

    return 0;
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

