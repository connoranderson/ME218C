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
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "StepService.h"
#include "ADService.h"
#include "PWM8Tiva.h"


/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define HALF_SEC (ONE_SEC/2)
#define TWO_SEC (ONE_SEC*2)
#define FIVE_SEC (ONE_SEC*5)
#define USE_4_BIT_WRITE 0x8000

#define NUMCHARS 49
#define MAX_INPUT_LENGTH 8


#define HALF_STEP_DRIVE 
#define WAVE_DRIVE	

#define NUM_CHANNELS 4

#define CLOCKWISE 0
#define COUNTER_CLOCKWISE 1

#define DRIFT_MAX_STEPS 100

#define CURRENT_DRIVE_MODE 4



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static StepState_t  CurrentState;
 
static int8_t currentStep = 0;

static uint8_t currentDirection = 0;

static uint8_t FullStepDrive[16] = {0xc5 , 0xc9 , 0xca , 0xc6 , 0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0 };
static uint8_t WaveStepDrive[16] = {0xc1 , 0xc8 , 0xc2 , 0xc4, 0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0};
static uint8_t HalfStepDrive[16] = {0xc5 , 0xc1 , 0xc9 , 0xc8 , 0xca , 0xc2 , 0xc6 , 0xc4,0x0, 0x0, 0x0, 0x0,0x0, 0x0, 0x0, 0x0};
static uint8_t MicroStepDrive[16] = {0x5 , 0x5 , 0x1 , 0x9 , 0x9 , 0x9 , 0x8 , 0xa , 0xa , 0xa , 0x2 , 0x6 , 0x6 , 0x6 , 0x4 , 0x5};

static uint8_t MicroStepDrive_PWM1[16] = {71 , 92 , 99 , 92 , 71 , 38 , 0 , 38 , 71 , 92 , 99 , 92 , 71 , 38, 0 , 38};
static uint8_t MicroStepDrive_PWM2[16] = {71 , 38, 0 , 38 , 71 , 92 , 99 , 92 , 71 , 38 , 0 , 38 , 71 , 92 , 99 , 92 };
														
static uint8_t *CurrentDrive;

static uint16_t driftSteps = 0;

static const uint16_t reqPeriod = 25000;
																 
static uint16_t NUM_STEPS; // 4 for FULL STEP AND WAVE STEP, 8 for HALF STEP, 16 for MICRO STEP
										 

										 
										 

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
bool InitStepService ( uint8_t Priority )
{
	puts("StepService Running");
	printf("\r\nNUM_STEPS %i",NUM_STEPS);
  ES_Event ThisEvent;
	
	switch(CURRENT_DRIVE_MODE){
		case 1 :
			CurrentDrive = FullStepDrive;
			NUM_STEPS = 4;
		break;
		case 2 :
			CurrentDrive = WaveStepDrive;
			NUM_STEPS = 4;
		break;
		case 3 :
			CurrentDrive = HalfStepDrive;
			NUM_STEPS = 8;
		break;
		case 4 :
			CurrentDrive = MicroStepDrive;
			NUM_STEPS = 16;
		break;
	}
	
  
	
  MyPriority = Priority;	// Initialize the MyPriority variable with the passed in parameter.
	
	if(NUM_STEPS > 8){
		// set frequency for arm servos (group 0, channel 0 & 1, PB6 & PB7)
		PWM8_TIVA_Init();
		PWM8_TIVA_SetPeriod(reqPeriod,0) ;		
		
	//	Initialize motor driver pins as digital H/L
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
    // Set Port B bits 0-3 to be a digital I/O pins
    HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    // Set bits 0-3 on Port B to be outputs
    HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	}else{	
//	Initialize motor driver pins as digital H/L
		// Set bit 1 and enable Port B
    HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
    // Wait until the peripheral reports that the clock is ready
    while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R1) != SYSCTL_PRGPIO_R1);
    // Set Port B bits 3-7 to be a digital I/O pins
    HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7);
    // Set bits 3-7 on Port B to be outputs
    HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7);
	}
	
// Set CurrentState to be Waiting4Char
	CurrentState = WaitingForNextStep;
	
	puts("Passed hw init");
	
	// Initialize a timer to generate events in run
  ES_Timer_InitTimer(STEP_TIMER, 20);

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
bool PostStepService( ES_Event ThisEvent )
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
ES_Event RunStepService( ES_Event ThisEvent )
{
	
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  uint16_t delayTime;


	
	
  switch (CurrentState){

    case WaitingForNextStep :
			if (ThisEvent.EventType == ES_BUTTON_PRESS){
				puts("Changing Directions \r\n");
				if(currentDirection == 0){
					currentDirection = 1;
				}else{
						currentDirection = 0;
				}					
			}
      if (ThisEvent.EventType == ES_TIMEOUT ){

				if(NUM_STEPS <= 8){ // If full, wave, or half stepping
					HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) =  *(CurrentDrive + currentStep);

				}else{ // If microstepping
					uint8_t preservePins = HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS));
					preservePins = preservePins & (BIT4HI | BIT5HI | BIT6HI | BIT7HI);
					HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) = preservePins | *(CurrentDrive + currentStep);
					PWM8_TIVA_SetDuty(MicroStepDrive_PWM1[currentStep],0);
					PWM8_TIVA_SetDuty(MicroStepDrive_PWM2[currentStep],1);
				}							
				if(currentDirection == 1){
					currentStep++;
				}else{
					currentStep--;
				}
				if(currentStep > (NUM_STEPS-1)){
					currentStep = 0;					
				}else if(currentStep < 0){
					currentStep = NUM_STEPS - 1;
				}
				ES_Timer_InitTimer(STEP_TIMER, GetStepperSpeed());
//				printf("\r\nCurrent Speed: %i" , GetStepperSpeed());
				
				// UNCOMMENT FOR TESTING FOR DRIFT
//				if(driftSteps < DRIFT_MAX_STEPS){
//					if(currentStep > (NUM_STEPS-1)){
//						currentStep = 0;					
//					}
//					HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) = CurrentDrive[currentStep];
//					currentStep++;
//					driftSteps++;
//					
//					ES_Timer_InitTimer(STEP_TIMER, 2);
//				}else{
//					if(currentStep < 0){
//						currentStep = (NUM_STEPS-1);
//					}	
//						HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) = CurrentDrive[currentStep];
//						currentStep--;		
//						driftSteps++;
//						ES_Timer_InitTimer(STEP_TIMER, 2);
//					}
//				if(driftSteps == 2*DRIFT_MAX_STEPS){
//					for(int i = 0; i < 10000000; i++);
//					driftSteps = 0;					
//				}
				
				//UNCOMMENT FOR MICROSTEPPING DRIFT
//								if(driftSteps < DRIFT_MAX_STEPS){
//					if(currentStep > (NUM_STEPS-1)){
//						currentStep = 0;					
//					}
//				uint8_t preservePins = HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS));
//				preservePins = preservePins & (BIT4HI | BIT5HI | BIT6HI | BIT7HI);
//				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) = preservePins | CurrentDrive[currentStep];
//				PWM8_TIVA_SetDuty(MicroStepDrive_PWM1[currentStep],0);
//				PWM8_TIVA_SetDuty(MicroStepDrive_PWM2[currentStep],1);
//					currentStep++;
//					driftSteps++;
//					
//					ES_Timer_InitTimer(STEP_TIMER,10);
//				}else{
//					if(currentStep < 0){
//						currentStep = (NUM_STEPS-1);
//					}	
//				uint8_t preservePins = HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS));
//				preservePins = preservePins & (BIT4HI | BIT5HI | BIT6HI | BIT7HI);
//				HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA+ALL_BITS)) = preservePins | CurrentDrive[currentStep];
//				PWM8_TIVA_SetDuty(MicroStepDrive_PWM1[currentStep],0);
//				PWM8_TIVA_SetDuty(MicroStepDrive_PWM2[currentStep],1);
//						currentStep--;		
//						driftSteps++;
//						ES_Timer_InitTimer(STEP_TIMER, 10);
//					}
//				if(driftSteps == 2*DRIFT_MAX_STEPS){
//					for(int i = 0; i < 10000000; i++);
//					driftSteps = 0;					
//				}
				
				


			}	
			
			
			
			
      break;



  }

  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

uint8_t getCurrentDriveMode(){
	
	return CURRENT_DRIVE_MODE;
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

