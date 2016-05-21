	errorlevel -302
	errorlevel -305
	list    p=PIC12F752
#include "p12F752.inc"

;
;		Code Protection OFF	(_CP_OFF)
;		Watchdog Timer OFF	(_WDTE_OFF)
;		Power Up Timer Enable ON	(_PWRTE_ON) WAITS TO RUN YOUR CODE TILL TIMER IS STABLE
;		MCLR Control OFF	(_MCLRE_OFF)
;		Oscillator Type INT	(_FOSC0_INT)
;		Write Project OFF	(_WRT_OFF)
		__config(_CP_OFF & _WDT_OFF & _PWRTE_ON & _FOSC_INT)
        ;__config (_CP_OFF & _WDTE_OFF & _PWRTE_ON & _MCLRE_OFF & _FOSC0_INT & _WRT_OFF)
       
;	
;		variable definitions
;
	
;Binary conventions


;Pins
BLUE_LED					equ		LATA0
RED_LED						equ		LATA1
COLOR						equ		RA2
DIM							equ		RA4
ENABLE						equ		RA5

;Variables
WREG_TEMP 					equ 	0x70
STATUS_TEMP 				equ 	0x71
PCLATH_TEMP 				equ 	0x72
SECONDS_LEFT				equ		0x73

;Constants
MAX_TIME					equ		0x2C

							org		0
							goto	Main
							org 	4
							goto 	ISR
							org		5

;
; Listing of LED Dimming Array
;
DIM_LED_ARRAY:
    addwf PCL, f
    RETLW 0x00
    RETLW 0x2F
    RETLW 0x4A
    RETLW 0x5D
    RETLW 0x6C
    RETLW 0x79
    RETLW 0x83
    RETLW 0x8C
    RETLW 0x94
    RETLW 0x9B
    RETLW 0xA1
    RETLW 0xA7
    RETLW 0xAD
    RETLW 0xB2
    RETLW 0xB6
    RETLW 0xBB
    RETLW 0xBF
    RETLW 0xC3
    RETLW 0xC6
    RETLW 0xCA
    RETLW 0xCD
    RETLW 0xD0
    RETLW 0xD3
    RETLW 0xD6
    RETLW 0xD9
    RETLW 0xDB
    RETLW 0xDE
    RETLW 0xE0
    RETLW 0xE3
    RETLW 0xE5
    RETLW 0xE7
    RETLW 0xE9
    RETLW 0xEB
    RETLW 0xED
    RETLW 0xEF
    RETLW 0xF1
    RETLW 0xF3
    RETLW 0xF5
    RETLW 0xF6
    RETLW 0xF8
    RETLW 0xFA
    RETLW 0xFB
    RETLW 0xFD
    RETLW 0xFE
    RETLW 0xFF


Main:

;Initialize ANSEL register
	banksel 	ANSELA
	BCF 		ANSELA, ANSA0
	BCF 		ANSELA, ANSA1
	BCF 		ANSELA, ANSA2
	BCF 		ANSELA, ANSA4
	BCF 		ANSELA, ANSA5

; Initialize pins 
	banksel 	LATA
	bcf 		LATA, LATA0
	bcf 		LATA, LATA1
	banksel 	TRISA
	BCF 		TRISA, TRISA0
	BCF 		TRISA, TRISA1


;Set up timer 0 with 2x prescaler (Bit value = 000)
	banksel 	OPTION_REG
	BCF 		OPTION_REG, T0CS
	BCF 		OPTION_REG, PSA
	BCF 		OPTION_REG, PS0
	BCF 		OPTION_REG, PS1
	BCF 		OPTION_REG, PS2

; Initialize the variables
	MOVLW	

;	Enable timer 0 interrupt.


; Enable global interrupts
	banksel 	INTCON
	bsf 		INTCON, GIE 

	goto $ ;Stay here after execution is done

	;######		END OF MAIN		#######


;	Interrupt Response Routine
ISR:
PUSH:
	movwf		WREG_TEMP	; save WREG
	movf		STATUS,W	; store STATUS in WREG
	clrf		STATUS		; change file register to bank 0
	movwf		STATUS_TEMP	; save STATUS value
	movf		PCLATH,W	; store PCLATH in WREG
	movwf		PCLATH_TEMP	; save PCLATH value
	clrf		PCLATH		; change to program memory page 0

ISR_BODY:
	; Check source of interrupt
	GOTO 		POP
	
POP: 
	clrf 		STATUS ;select bank 0
	movf 		PCLATH_TEMP,W ;store saved PCLATH value in WREG
	movwf 		PCLATH ;restore PCLATH
	movf 		STATUS_TEMP,W ;store saved STATUS value in WREG
	movwf 		STATUS ;restore STATUS
	swapf 		WREG_TEMP,F ;prepare WREG to be restored
	swapf 		WREG_TEMP,W ;restore WREG keeping STATUS bits
	retfie ;return from interrupt

	END