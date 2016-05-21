	
	list P=PIC12F752
	#include "p12F752.inc"
	__config (_CP_OFF & _WDT_OFF & _PWRTE_ON & _FOSC_INT & _MCLRE_OFF)
;	errorlevel -302	

    ORG     0            ; processor reset vector
    goto    main              ; go to beginning of program

    ORG     4             ; interrupt vector location
    goto 	ISR
	ORG     5
	
;
; Variable Defs
;

WREG_TEMP equ 1xF0 ; Temporary Registers in ALL Banks for Interrupts
STATUS_TEMP equ 1xF1
PCLATH_TEMP equ 1xF2
index equ 0x40 ; Index for stepping
WriteToPort equ 0x41 
DirectionInput equ RA4 ; RA4 Controls Direction
ModeInput equ RA5 ; RA5 Controls Mode

;
; Tables
;
FullStepDrive: ; Definitions for Stepper Steps
	addwf PCL, f
	retlw 0x03 ;STEP 1,0,1,0
	retlw 0x02 ;STEP 1,0,0,1
	retlw 0x00 ;STEP 0,1,0,1
	retlw 0x01 ;STEP 0,1,1,0
EndTBL:
	
main:
; ANSEL Definitions
	banksel ANSELA
	BCF ANSELA, ANSA0
	BCF ANSELA, ANSA1
	BCF ANSELA, ANSA2
	BCF ANSELA, ANSA4
	BCF ANSELA, ANSA5
	
; Pin Definitions (RA0 and RA1 are outputs)
	banksel TRISA
	BCF TRISA, TRISA0
	BCF TRISA, TRISA1
	banksel LATA
	bcf LATA, LATA0
	bcf LATA, LATA1

; Initialize Timer
	banksel INTCON
	bsf INTCON, GIE
	banksel PR2
	movlw 0xFF
	movwf PR2 ; Set PR2 to 255

	banksel T2CON
	bsf T2CON, T2OUTPS0 ; Postscale Timer2 to 16:1
	bsf T2CON, T2OUTPS1
	bsf T2CON, T2OUTPS2 
	bsf T2CON, T2OUTPS3 
;	bsf T2CON, T2CKPS1	; Prescale Timer2 to 16:1 
	bsf T2CON, TMR2ON ; Turn Timer ON
	
	banksel PIE1 ; Enable Timer2 Interrupt
	bsf PIE1, TMR2IE

; Initialize External Interrupts
	banksel INTCON
	bcf INTCON, INTF ; Clear interrupts before enabling
	bsf INTCON, PEIE
	bsf INTCON, INTE

	goto $

ISR:

PUSH:
	movwf WREG_TEMP ;save WREG
	movf STATUS,W ;store STATUS in WREG
	clrf STATUS ;change to file register bank0
	movwf STATUS_TEMP ;save STATUS value
	movf PCLATH,W ;store PCLATH in WREG
	movwf PCLATH_TEMP ;save PCLATH value
	clrf PCLATH ;change to program memory page0

ISR_BODY: 
;Figure out which interrupt we triggered
	banksel PIR1 
	btfsc PIR1, TMR2IF
		goto TimerInterrupt ; If Interrupt came from Timer
	banksel INTCON
	btfsc INTCON, INTF
		goto ExternalInterrupt ; If Interrupt came externally
	goto POP ; At this point, we should have recognized the interrupt, else skip to end

TimerInterrupt:
	bcf PIR1, TMR2IF ; Clear source of interrupt
	btfss PORTA, ModeInput ; Figure out which mode we're in
		goto POP;	; Ignore interrupt if not in timer mode
	call Step; ; Step if mode is set
	goto POP;

ExternalInterrupt:
	bcf INTCON, INTF ; Clear source of interrupt
	btfsc PORTA, ModeInput ; Figure out which mode we're in
		goto POP;	;  If mode bit is set, ignore button
	call Step ; Only step if mode is cleared
	goto POP


POP: 
	clrf STATUS ;select bank 0
	movf PCLATH_TEMP,W ;store saved PCLATH value in WREG
	movwf PCLATH ;restore PCLATH
	movf STATUS_TEMP,W ;store saved STATUS value in WREG
	movwf STATUS ;restore STATUS
	swapf WREG_TEMP,F ;prepare WREG to be restored
	swapf WREG_TEMP,W ;restore WREG keeping STATUS bits
	retfie ;return from interrupt

Step:
; Figure out which direction to step in
	btfss PORTA, DirectionInput
		goto Reverse
Forward	incf index,w
	andlw 0x03
	movwf index
	goto EndDirectionSet
Reverse
	decf index,w
	andlw 0x03
	movwf index

EndDirectionSet:
; Table lookup to find next step value
	movlw FullStepDrive>>8 ; Just in case the table is on another page
	movwf PCLATH
	movf index, w
	call FullStepDrive
; Write w to port
	movwf WriteToPort ; Temporarily store the w register
	movlw 0xFC
	andwf LATA, f ; Clear two output bits of LATA
	movf WriteToPort, w	
	IORWF LATA, f ; Write value to port
	return

; Subroutine used for debugging interrupts
Toggle: 
	banksel LATA	
	btfss LATA, LATA0
		goto Clear
	bcf LATA,LATA0
	goto Done
Clear;
	bsf LATA,LATA0
Done:
	return
	

; REQUIRED - tell assembler this is the end of source code
	end
