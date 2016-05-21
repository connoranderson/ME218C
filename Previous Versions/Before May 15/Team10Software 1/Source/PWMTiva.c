#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"

#include "PWMTiva.h"

static uint32_t ulPeriod0;
static uint32_t ulPeriod1;
static uint8_t  duty0;
static uint8_t  duty1;
static uint8_t  duty2;
static uint8_t  duty3;

void PWM_TIVA_Init(void)
{    

   //Configure PWM Clock to system / 32
   SysCtlPWMClockSet(SYSCTL_PWMDIV_32);

   // Enable the peripherals used by this program.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);  //The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins
    // Calculate the period for 500 Hz including divide by 32 in PWM clock
    ulPeriod0 = SysCtlClockGet()/32 / 500; //PWM frequency 500HZ
    ulPeriod1 = ulPeriod0;
  
    //Configure PB6,PB7,PB4, PB5 Pins as PWM
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinConfigure(GPIO_PB7_M0PWM1);
    GPIOPinConfigure(GPIO_PB4_M0PWM2);
    GPIOPinConfigure(GPIO_PB5_M0PWM3);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    //Configure PWM Options
    //PWM_GEN_0 Covers M0PWM0 and M0PWM1
    //PWM_GEN_1 Covers M0PWM2 and M0PWM3 See page 207 4/11/13 DriverLib doc
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC); 
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC); 

    //Set the Period (expressed in clock ticks)
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod0);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ulPeriod1);

    //Set PWM duty to 0 initially
    PWM_TIVA_SetDuty( 0, 0);
    PWM_TIVA_SetDuty( 0, 1);
    PWM_TIVA_SetDuty( 0, 2);
    PWM_TIVA_SetDuty( 0, 3);

    // Enable the PWM generator
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);

    // Turn on the Output pins
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT | PWM_OUT_2_BIT | PWM_OUT_3_BIT, true);

}

void PWM_TIVA_SetDuty( uint8_t dutyCycle, uint8_t channel)
{
  uint32_t updateVal;
  
  if ((0 == dutyCycle) || ( dutyCycle >= 100)){ // don't try to calculate with 0 DC
    updateVal = 0;
  }else{ // reasonable duty cycle number
    if (channel < 2){ // group 0
      updateVal = (ulPeriod0*dutyCycle)/100;
    }else{
      updateVal = (ulPeriod1*dutyCycle)/100;
    }
  }
  switch (channel){
    case 0:
    duty0 = dutyCycle; 
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,updateVal);     
    break;
    
    case 1:
     duty1 = dutyCycle; 
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,updateVal);     
      
    break;
    
    case 2:
     duty2 = dutyCycle; 
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,updateVal);     
      
    break;
    
    case 3:
     duty3 = dutyCycle; 
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,updateVal);     
      
    break;
  }
}

void PWM_TIVA_SetPulseWidth( uint16_t NewPW, uint8_t channel)
{
   
  switch (channel){
    case 0:
    if ( NewPW < ulPeriod0)
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,NewPW);     
    break;
    
    case 1:
     if ( NewPW < ulPeriod0) 
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,NewPW);          
    break;
    
    case 2:
     if ( NewPW < ulPeriod1) 
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,NewPW);           
    break;
    
    case 3:
     if ( NewPW < ulPeriod1) 
      PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,NewPW);           
    break;
  }
}

/*****************************************************************************
  PWM_TIVA_SetPeriod( uint16_t reqPeriod, uint8_t group)
    sets the requested PWM group's period to the Requested Period
    group 0 = channels 0 & 1
    group 2 = channels 2 & 3
*****************************************************************************/

void PWM_TIVA_SetPeriod( uint16_t reqPeriod, uint8_t group)
{
    //Set the Period (expressed in clock ticks)
  if( 0 == group){
   ulPeriod0 = reqPeriod;
   PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod0); 
   PWM_TIVA_SetDuty( duty0, 0);
   PWM_TIVA_SetDuty( duty1, 1);
}else{
    if( 1 == group){
      ulPeriod1 = reqPeriod;
      PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ulPeriod1); 
      PWM_TIVA_SetDuty( duty2, 2);
      PWM_TIVA_SetDuty( duty3, 3);
      
    }
  }
 }

 /*****************************************************************************
  PWM_TIVA_SetFreq( uint16_t reqPeriod, uint8_t group)
    sets the requested PWM group's frequency to the Requested Frequency, in Hz
    group 0 = channels 0 & 1
    group 2 = channels 2 & 3
*****************************************************************************/

void PWM_TIVA_SetFreq( uint16_t reqFreq, uint8_t group)
{
    //Set the Frequency (expressed in Hz)
  if( 0 == group){
   ulPeriod0 = SysCtlClockGet()/32 /reqFreq;
   PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, ulPeriod0); 
   PWM_TIVA_SetDuty( duty0, 0);
   PWM_TIVA_SetDuty( duty1, 1);
  }else{
    if( 1 == group){
      ulPeriod1 = SysCtlClockGet()/32 /reqFreq;
      PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, ulPeriod1); 
      PWM_TIVA_SetDuty( duty2, 2);
      PWM_TIVA_SetDuty( duty3, 3);
     
    }
  }
 }

