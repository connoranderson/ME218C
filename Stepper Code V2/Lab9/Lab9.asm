	
	list P=PIC12F752
	#include "p12F752.inc"
	__config (_CP_OFF & _WDT_OFF & _PWRTE_ON & _FOSC_INT)
	errorlevel -302	

    ORG     0x00            ; processor reset vector
    goto    main              ; go to beginning of program
    

    ORG     0x004             ; interrupt vector location
    goto ISR
    retfie                    ; return from interrupt
	
;
; Variable Defs
;


;
; Tables
;
FullStepDrive:
	addwf PCL,f
	retlw 0x03 ;STEP 1,0,1,0
	retlw 0x02 ;STEP 1,0,0,1
	retlw 0x00 ;STEP 0,1,0,1
	retlw 0x01 ;STEP 0,1,1,0
	
main:
; ANSEL Definitions
	banksel ANSELA
	BCF ANSELA, ANSA0
	BCF ANSELA, ANSA1
	BCF ANSELA, ANSA2
	BCF ANSELA, ANSA4
	BCF ANSELA, ANSA5
	
; Pin Definitions
	MOVLW 0x03
	ANDWF PORTA, f
	
	BCF TRISA, TRISA0
	BCF TRISA, TRISA1

	
	goto $

ISR:




; REQUIRED - tell assembler this is the end of source code
	end