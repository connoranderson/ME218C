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
#include "inc/hw_Timer.h"
#include "inc/hw_pwm.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "DCService.h"
#include "ADService.h"
#include "PWM_Module.h"


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define USE_4_BIT_WRITE 0x8000

#define NUMCHARS 49
#define MAX_INPUT_LENGTH 8

#define NUM_CHANNELS 4

#define OPEN_BRIDGE 0x1

#define POT_TO_RPM 74
#define TicksPerMS 40000

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

void MotorHWInit( void);
void PeriodicInterruptInit( void);
void PeriodicIntResponse( void );

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static DCState_t  CurrentState;

static const uint16_t reqPeriod = 25000;

static uint32_t TimeoutCount = 0;

static int32_t integral = 0;

static float kp = 0.6;
static float ki = 0.1;
																 									 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLCDService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
    Connor Anderson, 10/25/2015
****************************************************************************/
bool InitDCService ( uint8_t Priority )
{
	puts("DCService Running");
  ES_Event ThisEvent;
  
	// Initialize PWM here
	InitPWM();
	
	MotorHWInit();
	
//	// Periodic Interrupt
	PeriodicInterruptInit();
	
  MyPriority = Priority;	// Initialize the MyPriority variable with the passed in parameter.	
	
// Set CurrentState to be Waiting4Char
	CurrentState = Driving;
	
	puts("Passed init");
	
	// Initialize a timer to generate events in run
  ES_Timer_InitTimer(MOTOR_TIMER, 20);

// End of InitializeButton (return True)
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostLCDService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     Connor Anderson, 10/25/2015
****************************************************************************/
bool PostDCService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunLCDService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   Connor Anderson, 10/25/2015
****************************************************************************/
ES_Event RunDCService( ES_Event ThisEvent )
{
	
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  uint16_t delayTime;

	static uint16_t counter = 0;
	
	
  switch (CurrentState){

    case Driving :

      if (ThisEvent.EventType == ES_TIMEOUT ){
				 // PWM SET DUTY HERE
//				SetDuty(GetMotorSpeed());
//				ES_Timer_InitTimer(MOTOR_TIMER, 100);	
				
				// USED FOR GETTING RPM DC TRANSFER FNCT
//				printf("\r\nCounter Value: %d" , counter);
//				SetDuty(counter*5);
//				counter++;
//				if(counter==21){
//					counter = 0;
//				}				
//				ES_Timer_InitTimer(MOTOR_TIMER, 7000);	
			}	

      break;
  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

void MotorHWInit( void){
	//	Initialize motor driver pins as digital H/L
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
    // Set Port B bits 0-3 to be a digital I/O pins
    HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1);
    // Set bits 0-3 on Port B to be outputs
    HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_0 | GPIO_PIN_1);
		// Set 0 low and 1 high
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) =  OPEN_BRIDGE;	
	
}

void PeriodicInterruptInit( void){
	volatile uint32_t Dummy; // use volatile to avoid over-optimization
	// start by enabling the clock to the timer (Wide Timer 1)
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R1; // kill a few cycles to let the clock get going
	Dummy = HWREG(SYSCTL_RCGCGPIO);
	// make sure that timer (Timer A) is disabled before configuring
	HWREG(WTIMER1_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	// set it up in 32bit wide (individual, not concatenated) mode
	HWREG(WTIMER1_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	// set up timer A in periodic mode so that it repeats the time-outs
	HWREG(WTIMER1_BASE+TIMER_O_TAMR) =
	(HWREG(WTIMER1_BASE+TIMER_O_TAMR)& ~TIMER_TAMR_TAMR_M)|
	TIMER_TAMR_TAMR_PERIOD;
	// set timeout to 100mS
	HWREG(WTIMER1_BASE+TIMER_O_TAILR) = TicksPerMS * 100;
	// enable a local timeout interrupt
	HWREG(WTIMER1_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
	// enable the Timer A in Wide Timer 1 interrupt in the NVIC
	// it is interrupt number 96 so appears in EN3 at bit 0
	HWREG(NVIC_EN3) = BIT0HI;
	// Set priority of interrupt 0-7 where lower number is higher priority
	HWREG(NVIC_PRI24 + NVIC_PRI24_INTA_S) = 7;
	// Unmask interrupt if it's not already
	HWREG(NVIC_PRI24) |= NVIC_PRI24_INTA_M;
	// make sure interrupts are enabled globally
	__enable_irq();
	// now kick the timer off by enabling it and enabling the timer to
	// stall while stopped by the debugger
	HWREG(WTIMER1_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN |
	TIMER_CTL_TASTALL);
}

void PeriodicIntResponse( void ){
	// start by clearing the source of the interrupt
	HWREG(WTIMER1_BASE+TIMER_O_ICR) = TIMER_ICR_TATOCINT;
	
	uint16_t desiredRPM = GetPotValue()/POT_TO_RPM; // Convert pot value to an RPM
	
	// PI Control
	int16_t error = (desiredRPM - GetCurrentSpeed());
	int32_t driveSignal = kp * error + ki*integral;
	integral += error;
	
	// Anti Windup
	if(driveSignal > 100){	
		driveSignal = 100;
		if(error > 0){
			integral -= error;
		}
	}else if(driveSignal < 0){
		driveSignal = 0;
		if(error < 0){
			integral -= error;
		}
	}
	
	printf("\r\n Desired RPM: %d  Error: %d  Duty Cycle: %d",desiredRPM, error,driveSignal);
	
	SetDuty(driveSignal);		
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


