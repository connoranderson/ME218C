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


#define PeriodInMS 1 // Assumes 200Hz clock
#define PeriodInHMicroS 1 // Assumes 200Hz clock
#define PWMTicksPerMS 40000/32 
#define PWMTicksPerHMicroS 4000/32 
#define BitsPerNibble 4




bool InitPWM ( void ){
	
// start by enabling the clock to the PWM Module (PWM0)
HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	
// enable the clock to Port B
HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	
// Select the PWM clock as System Clock/32
HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
(SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	
// make sure that the PWM module clock has gotten going
while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
;
// disable the PWM while initializing
HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;

// program generator A to go to 1 at rising compare A, 0 on falling compare A
HWREG( PWM0_BASE+PWM_O_0_GENA) =
(PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO );

// program generator B to go to 1 at rising compare B, 0 on falling compare B
HWREG( PWM0_BASE+PWM_O_0_GENB) =
(PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO );

// Set the PWM period. Since we are counting both up & down, we initialize
// the load register to 1/2 the desired total period. We will also program
// the match compare registers to 1/2 the desired high time
HWREG( PWM0_BASE+PWM_O_0_LOAD) = ((PeriodInHMicroS * PWMTicksPerHMicroS))>>1;

// Set the initial Duty cycle on A to 50% by programming the compare value
// to 1/2 the period to count up (or down). Technically, the value to program
// should be Period/2 - DesiredHighTime/2, but since the desired high time is 1/2
// the period, we can skip the subtract
HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD)>>1;

// Set the initial Duty cycle on B to 25% by programming the compare value
// to Period/2 - Period/8 (75% of the period)
HWREG( PWM0_BASE+PWM_O_0_CMPB) = (HWREG( PWM0_BASE+PWM_O_0_LOAD)) -
(((PeriodInMS * PWMTicksPerMS))>>3);

// enable the PWM outputs
HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);

// now configure the Port B pins to be PWM outputs
// start by selecting the alternate function for PB6 & 7
HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);

// now choose to map PWM to those pins, this is a mux value of 4 that we
// want to use for specifying the function on bits 6 & 7
HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) =
(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00fffff) + (4<<(7*BitsPerNibble)) +
(4<<(6*BitsPerNibble));

// Enable pins 6 & 7 on Port B for digital I/O
HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI);

// make pins 6 & 7 on Port B into outputs
HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (BIT7HI |BIT6HI);

// set the up/down count mode, enable the PWM generator and make
// both generator updates locally synchronized to zero count
HWREG(PWM0_BASE+ PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
	
}

bool SetDuty( uint8_t dutyCycle ){
	if(dutyCycle == 100 || dutyCycle > 100){
		dutyCycle = 99;
	}
	if(dutyCycle == 0){
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD);
	}else{
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD)-HWREG( PWM0_BASE+PWM_O_0_LOAD)*(dutyCycle)/100;
	}
}
	


