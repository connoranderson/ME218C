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
#include "EncoderService.h"


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

#define PERIOD_TO_RPM 794492
#define TicksPerMS 40000

#define RPM_TO_BARS 57


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

void EncoderHWInit( void);
void InputCaptureInit( void);
void WriteLEDS( uint8_t input);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static EncoderState_t CurrentState;

static uint32_t Period;
static uint32_t LastCapture;
static uint32_t CurrentSpeed;
static bool encoderChange = false;
																 									 

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
bool InitEncoderService ( uint8_t Priority )
{
	puts("EncoderService Running");
  ES_Event ThisEvent;
  
	// Encoder HW Init
	EncoderHWInit();
  // Input Capture Init
	InputCaptureInit();
  // Start Timer
	
  MyPriority = Priority;	// Initialize the MyPriority variable with the passed in parameter.	
	
// Set CurrentState to be Waiting4Char
	CurrentState = Finding_Period;
	
	puts("Passed init");
	
	// Initialize a timer to generate events in run
  ES_Timer_InitTimer(ENCODER_TIMER, 20);

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
bool PostEncoderService( ES_Event ThisEvent )
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
ES_Event RunEncoderService( ES_Event ThisEvent )
{
	
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  uint16_t delayTime;


	
	
  switch (CurrentState){

    case Finding_Period:

      if (ThisEvent.EventType == ES_TIMEOUT ){
				// convert period into RPMs and store in module variable
				
        CurrentSpeed = PERIOD_TO_RPM/Period;
				
        // Write RPM variable to TeraTerm window
//				printf("\r\n%d RPM",  CurrentSpeed);
				
				if(!encoderChange){ // Timeout for if we are stopped
					Period = 0;		
					CurrentSpeed = 0;
				}
				
        // Write to LEDs				
				if(encoderChange){
					uint8_t newLEDValue = (RPM_TO_BARS-CurrentSpeed)/7;
//					printf("\r\n%d LED",  newLEDValue);
					WriteLEDS(newLEDValue); 
					encoderChange = false;
				}
				
        // Restart timer
				ES_Timer_InitTimer(ENCODER_TIMER, 100);
			}	

      break;
  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

void EncoderHWInit( void){
  // Initialize LED Pins as Output
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//Manual delay to ensure clock is ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R3) != SYSCTL_PRGPIO_R3);
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) |= (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
	//Manual delay to ensure clock is ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R0) != SYSCTL_PRGPIO_R0);
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= (GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

	// Initialize LED Pins as Output
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN) |= (GPIO_PIN_2);
	//Manual delay to ensure clock is ready
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R4) != SYSCTL_PRGPIO_R4);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR) |= (GPIO_PIN_2);
	HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS)) |= (GPIO_PIN_2);
	
	HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
	HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO);//Set all LEDs LO for tiny interval

		
}

void InputCaptureInit( void){
      // start by enabling the clock to the timer (Wide Timer 0)
    HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R0;
    // enable the clock to Port E
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R2;
    // since we added this Port E clock init, we can immediately start
    // into configuring the timer, no need for further delay
    // make sure that timer (Timer A) is disabled before configuring
    HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
    // set it up in 32bit wide (individual, not concatenated) mode
    // the constant name derives from the 16/32 bit timer, but this is a 32/64
    // bit timer so we are setting the 32bit mode
    HWREG(WTIMER0_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
    // we want to use the full 32 bit count, so initialize the Interval Load
    // register to 0xffff.ffff (its default value :-)
    HWREG(WTIMER0_BASE+TIMER_O_TAILR) = 0xffffffff;
    // set up timer A in capture mode (TAMR=3, TAAMS = 0),
    // for edge time (TACMR = 1) and up-counting (TACDIR = 1)
    HWREG(WTIMER0_BASE+TIMER_O_TAMR) =
    (HWREG(WTIMER0_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) |
    (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
    // To set the event to rising edge, we need to modify the TAEVENT bits
    // in GPTMCTL. Rising edge = 00, so we clear the TAEVENT bits
    HWREG(WTIMER0_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
    // Now Set up the port to do the capture (clock was enabled earlier)
    // start by setting the alternate function for Port E bit 4 (WT0CCP0)
    HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= BIT4HI;
    // Then, map bit 4's alternate function to WT0CCP0
    // 7 is the mux value to select WT0CCP0, 16 to shift it over to the
    // right nibble for bit 4 (4 bits/nibble * 4 bits)
    HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) =
    (HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (7<<16);
    // Enable pin on Port E for digital I/O
    HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= BIT4HI;
    // make pin 4 on Port E into an input
    HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= BIT4LO;
    // back to the timer to enable a local capture interrupt
    HWREG(WTIMER0_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
    // enable the Timer A in Wide Timer 0 interrupt in the NVIC
    // it is interrupt number 94 so appears in EN2 at bit 30
    HWREG(NVIC_EN2) |= BIT30HI;
    // make sure interrupts are enabled globally
    __enable_irq();
    // now kick the timer off by enabling it and enabling the timer to
    // stall while stopped by the debugger
    HWREG(WTIMER0_BASE+TIMER_O_CTL) |= (TIMER_CTL_TAEN | TIMER_CTL_TASTALL);
  
}

void InputCaptureResponse( void ){
	
  uint32_t ThisCapture;
  // start by clearing the source of the interrupt, the input capture event
  HWREG(WTIMER0_BASE+TIMER_O_ICR) = TIMER_ICR_CAECINT;
  // now grab the captured value and calculate the period
  ThisCapture = HWREG(WTIMER0_BASE+TIMER_O_TAR);
  Period = ThisCapture - LastCapture;
  // update LastCapture to prepare for the next edge
  LastCapture = ThisCapture;
	// update encoderChange variable for LED Display
	encoderChange = true;
}

void WriteLEDS( uint8_t input){
  switch (input){

    case 0:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI); //Set Level 5 HI
      break;
		case 1:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI); //Set Level 5 HI
      break;
		case 2:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI); //Set Level 5 HI
      break;
		case 3:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI); //Set Level 5 HI
      break;
		case 4:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI); //Set Level 5 HI
      break;
		case 5:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI); //Set Level 5 HI
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT2HI); //Set Level 5 HI
      break;
		case 6:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI); //Set Level 5 HI
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT2HI| BIT3HI ); //Set Level 5 HI
      break;
		case 7:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI); //Set Level 5 HI
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT2HI| BIT3HI | BIT4HI); //Set Level 5 HI
      break;
		case 8:
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT0LO & BIT1LO & BIT2LO & BIT3LO); //Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) &= (BIT2LO & BIT3LO & BIT4LO & BIT5LO);//Set all LEDs LO for tiny interval
			HWREG(GPIO_PORTD_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI); //Set Level 5 HI
			HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA+ALL_BITS)) |= (BIT2HI| BIT3HI | BIT4HI | BIT5HI); //Set Level 5 HI
      break;
	}
}

int32_t GetCurrentSpeed(void){
	return CurrentSpeed;
}






/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

