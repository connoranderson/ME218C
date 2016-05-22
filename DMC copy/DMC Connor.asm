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
WAITING						equ		0
RECEIVING					equ		1
STEP						equ		1
OFF							equ		0
ON							equ		1

;Pins
LINE_IN						equ		RA0
RED_LED						equ		LATA1
BLUE_LED					equ		LATA2
STEPPER_DIR					equ		LATA4
STEPPER_EN					equ		LATA5

;Variables
PACKET						equ		0x70
MOTOR_STATE					equ		0x71
LED_STATE					equ		0x72
BITS_LEFT					equ		0x73
COUNTS_LEFT					equ		0x74

WREG_TEMP 					equ 	0x75
STATUS_TEMP 				equ 	0x76
PCLATH_TEMP 				equ 	0x77

PACKET_COPY					equ		0x78
IS_PAIRED					equ		0x79
COLOR						equ		0x7A
LED_ON						equ		0x7B
LED_Timeval					equ		0x7C
LED_Counter					equ		0x7D
LED_Lowerval				equ		0x7E
LED_Upperval				equ		0x7F


;Constants
BITS_IN_PACKET				equ		0x3	
MAX_COUNTS					equ		0x5A


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
	banksel ANSELA
	BCF ANSELA, ANSA0
	BCF ANSELA, ANSA1
	BCF ANSELA, ANSA2
	BCF ANSELA, ANSA4
	BCF ANSELA, ANSA5

; Initialize pins 
; RA1, RA2, RA4, and RA5 --> Outputs
; RA0 --> Input
	banksel LATA
	bcf LATA, LATA1
	bcf LATA, LATA2
	bcf LATA, LATA4
	bcf LATA, LATA5
	banksel TRISA
	BCF TRISA, TRISA1
	BCF TRISA, TRISA2
	BCF TRISA, TRISA4
	BCF TRISA, TRISA5


;Set up timer 0 with 2x prescaler (Bit value = 000)
	banksel OPTION_REG
	BCF OPTION_REG, T0CS
	BCF OPTION_REG, PSA
	BCF OPTION_REG, PS0
	BCF OPTION_REG, PS1
	BCF OPTION_REG, PS2

; Set up timer 1 for the stepper + LEDs
    banksel T1CON
    BCF T1CON, TMR1CS1 ; Set the source to instruction clock
    BCF T1CON, TMR1CS0 
    BSF T1CON, T1CKPS1 ; Set the prescaler to 4. 
    BCF T1CON, T1CKPS0 ; For 8, change BCF to BSF
    BCF T1GCON, TMR1GE ; Disable the gate control

;########## PWM ##########
;   Set up timer 2 for the communication protocol
;    movlw       0xFA        ; Move 0xFA the working register 250 to reach 1ms
;    banksel     PR2     ; Select the bank with PR2
;    movwf       PR2     ; Move 0xFA to PR2 so that the timer counts through 250 (not including pre or post scalers)
                    ; Leave the default T2CON register to have a 1:1 postscaler and 1:1 prescaler with the timer off

; 	Setting up timer 2
	movlw 		0xFF			; Move 0xFF to the working register
	banksel 	PR2				; Select the bank with PR2
	movwf 		PR2				; Move 0xFF to PR2 so that the timer counts through 255 (not including pre or post scalers)
;	movlw 		0x07			; Move 0x07 to the working register
	banksel 	T2CON			; Select the bank with the t2con register
;	movwf 		T2CON			; Move 0x07 into the T2CON register to have a 1:1 postscaler and 1:16 prescaler
	BSF			T2CON, 3
	BSF			T2CON, 4
	BSF			T2CON, 5
	BSF			T2CON, 6
;	BSF			T2CON, 1
;	banksel		PIR1
;	bcf			PIR1, TMR2IF	; 
;	banksel 	PIE1			; Select the bank with the PIE1 register
;	bsf 		PIE1, TMR2IE	; Enable the timer 2 interrupt 

; Initialize the variables
	movlw		BITS_IN_PACKET
	movwf		BITS_LEFT
	movlw		MAX_COUNTS
	movwf		COUNTS_LEFT
	movlw		0x00
	movwf		IS_PAIRED
	movwf		LED_ON

;	Set up the interrupt on rising edge on RA0
	banksel		INTCON		; Select the bank with the intcon register
	bsf		INTCON, GPIE	; Enable the interrupt on change interrupts
	banksel 	IOCAF
	bcf		IOCAF, IOCAF0
	banksel		IOCAP		; Select the bank with the IOCAP register
	bsf		IOCAP, IOCAP0	; Enable the interrupt on positive edge

; Enable External Interrupts
	banksel INTCON
	bsf INTCON, PEIE

; Enable global interrupts
	banksel INTCON
	bsf INTCON, GIE 

	goto $ ;Stay here after execution is done

	;######		END OF MAIN		#######


;	Interrupt Response Routine
ISR:
PUSH	movwf	WREG_TEMP	; save WREG
		movf	STATUS,W	; store STATUS in WREG
		clrf	STATUS		; change file register to bank 0
		movwf	STATUS_TEMP	; save STATUS value
		movf	PCLATH,W	; store PCLATH in WREG
		movwf	PCLATH_TEMP	; save PCLATH value
		clrf	PCLATH		; change to program memory page 0

ISR_BODY:
	; Check source of interrupt

TIMER2_CHECK:
		banksel 	PIR1 			; Select the bank with the PIR1 register
		btfsc 		PIR1, TMR2IF    ; Check to see if Timer 2 caused the interrupt, skip if not
		GOTO		PWM	; Go to PWM

		banksel IOCAF	;Interrupt on rising edge
		btfsc IOCAF, IOCAF0		; If Interrupt was from Start bit (Edge)
		call START_BIT; Call start bit response routine


		banksel	INTCON
		btfss	INTCON, TMR0IE	;Skip the timer 0 check if its interrupt is disabled.
		goto	TIMER1_CHECK
	
		btfsc INTCON, TMR0IF		; If Interrupt was from comms timer.
		call RECEIVE; Call timer 0 response routine

TIMER1_CHECK:	
		banksel		PIR1		; Select the bank with the PIR1 register
		btfsc		PIR1, TMR1IF	; Check to see if Timer 1 caused the interrupt, skip if not
		call		TIMER1_INTERRUPTSOURCE	; Go to LED_TIMEOUT
		
		GOTO POP

;This is the waiting mode of the receive SM.
START_BIT:

	;Set up TMR0 for half the bit period.
	banksel OPTION_REG
	BCF OPTION_REG, T0CS
	BCF OPTION_REG, PSA
	BCF OPTION_REG, PS0
	BCF OPTION_REG, PS1
	BCF OPTION_REG, PS2
	movlw			0xD2	;Half bit period away from rollover
	banksel 		TMR0
	movwf			TMR0
	
	banksel 	IOCAF
	; Clear and disable the RA0 rising edge interrupt
	bcf IOCAF, IOCAF0
	banksel		IOCAP
	bcf IOCAP, IOCAP0
	banksel INTCON
	bcf INTCON, RAIF ;Clear the RA change interrupt flag in INTCON
	bcf INTCON, GPIE ;Disable the RA change int.
	
	;Shift the variable 1 left and read the pin.
	banksel		PORTA
	rlf			PACKET
	bcf			PACKET, 0
	btfsc		PORTA, LINE_IN
	bsf			PACKET, 0
	
	;Unmask the timer 0 interrupt.
	banksel 	INTCON
	bcf			INTCON,TMR0IF
	bsf			INTCON,TMR0IE 

	;TEST pulse bit2 hi for one cycle
	;banksel		LATA
	;BSF			LATA, LATA2
	;bcf			LATA, LATA2
	
	RETURN

;This deals with filling up the xceive variable.
RECEIVE:
	
	;Shift the variable 1 left and read the pin.
	banksel		PORTA
	rlf			PACKET
	bcf			PACKET, 0
	btfsc		PORTA, LINE_IN
	bsf			PACKET, 0
	
	;Set up TMR0 for the bit period.
	banksel OPTION_REG
	BCF OPTION_REG, T0CS
	BCF OPTION_REG, PSA
	BCF OPTION_REG, PS0
	BCF OPTION_REG, PS1
	BCF OPTION_REG, PS2
	movlw			0x99	;Full bit period away from rollover.
	banksel 		TMR0
	movwf			TMR0
	
	;Clear the timer 0 interrupt
	banksel 	INTCON
	bcf			INTCON,TMR0IF
	bsf			INTCON,TMR0IE

;	;TEST PULSE LATA2 for 2 cycles
	;banksel LATA
	;bsf		LATA, LATA2
	;NOP
	;bcf		LATA, LATA2

	;Decrement bits left counter, skip if zero
	DECFSZ		BITS_LEFT
	RETURN
	goto		LAST_BIT_READ

;This part of the code interprets the transmission and takes actions based on it.
LAST_BIT_READ:

;Disable timer 0 interrupt
	banksel 	INTCON
	bcf			INTCON,TMR0IE

;Reset transmission counter.
	MOVLW		BITS_IN_PACKET
	MOVWF		BITS_LEFT

; Reenable the rising edge interrupt
;	Set up the interrupt on rising edge on RA0
	banksel		INTCON		; Select the bank with the intcon register
	bsf		INTCON, GPIE	; Enable the interrupt on change interrupts
	banksel 	IOCAF
	bcf		IOCAF, IOCAF0
	banksel		IOCAP		; Select the bank with the IOCAP register
	bsf		IOCAP, IOCAP0	; Enable the interrupt on positive edge
	
	;Check whether the status has changed.
	movf	PACKET, W
	movwf	PACKET_COPY
	RRF		PACKET_COPY
	movf	PACKET_COPY, W
	andlw	0x01
	SUBWF	IS_PAIRED, W
	BTFSS PACKET, 2 ; See if input is noisy (ie a start bit of 0), ignore if it is
		RETURN
	BTFSS	STATUS, Z ;If the things are the same, don't change anything
	CALL	START_MOVING

	;Set the LED color variable accordingly.
	CLRF	COLOR
	movf	PACKET, W
	ANDLW	0x01
	BTFSS	STATUS, Z
	BSF		COLOR, 0

	RETURN
	
START_MOVING:
		;First update the pairing status.
		movf		IS_PAIRED, W
		xorlw		0x01
		movwf		IS_PAIRED

		;Then determine which way to go.
		banksel 	LATA
		BTFSC		IS_PAIRED, 0
		BSF			LATA, STEPPER_DIR	
		BTFSS		IS_PAIRED, 0
		BCF			LATA, STEPPER_DIR
		
		;Change the motor status to moving.
		BSF			MOTOR_STATE, 0
		
		; Set up timer 1 with the required motor motion time
		banksel		T1CON		; Select the bank with the T1CON register
		bcf		T1CON, TMR1ON	; Turn timer 1 off
		BCF T1CON, TMR1CS1 ; Set the source to instruction clock
		BCF T1CON, TMR1CS0 
		BSF T1CON, T1CKPS1 ; Set the prescaler to 4
		BCF T1CON, T1CKPS0 
		BCF T1GCON, TMR1GE ; Disable the gate control

;		;Check if the motor was moving.
;		BTFSC		STEPPER_EN, 0
;		GOTO 		MID_STEP		

;When not currently stepping, set the default timer value.
;NOT_MID_STEP:
		banksel TMR1L ; First set the TMR1 value to 0
		MOVLW 	0x00
		MOVWF 	TMR1L
		MOVWF 	TMR1H
;		GOTO	T1_ENABLE_INTERRUPT

;If direction is changed mid-step, the timer must be set shorter not to overrun the mechanism.
;Do a crappy two's complement. Not really tested for robustness.
;MID_STEP:
;		banksel TMR1L
;		movlw	0xFF
;		subwf	TMR1H, F
;		subwf	TMR1L, F
		
T1_ENABLE_INTERRUPT:
		banksel PIE1 ; Prepare to start timer 1
		BSF PIE1, TMR1IE ; Set the timer 1 interrupt enable
		banksel T1CON
		BSF T1CON, TMR1ON ; Turn on timer 1
		banksel PIR1 ; Then clear the T1 interrupt flag
		BCF PIR1, TMR1IF
		
		;Activate the motor.
		banksel	LATA
		BSF LATA, STEPPER_EN
		RETURN

TIMER1_INTERRUPTSOURCE:
		;	Disable timer 1
		;banksel		T1CON		; Select the bank with the T1CON register
		;bcf		T1CON, TMR1ON	; Turn timer 1 off
		banksel 	PIR1		; Select the bank with the PIR1 register
		bcf 		PIR1, TMR1IF	; Clear the Timer 1 interrupt flag
		;banksel		PIE1		; Select the bank with the INTCON register
		;bcf		PIE1, TMR1IE	; Disable the timer 1 interrupt

	;banksel LATA
	;bsf LATA, BLUE_LED

		;Determine timer 1 interrup source. Motor vs. LEDs
		btfsc MOTOR_STATE, 0 ;If MOTOR_STATE is set (Info is on bit 0)
		CALL STOP_MOVING ;Stop moving. Make sure it doesnt return back here
		CALL DIM_LEDS ;Timer 1 interrupt was to turn off LEDs
		RETURN

STOP_MOVING:
		banksel		T1CON		; Select the bank with the T1CON register
		bcf		T1CON, TMR1ON	; Turn timer 1 off
		banksel	LATA
		BCF LATA, STEPPER_EN ;Clear bit 0 in stepper enable
		BCF MOTOR_STATE,0 ;Set motor state to not moving
		BTFSS PACKET, 2 ; See if start bit is set
			RETURN
		BTFSC	IS_PAIRED, 0 ; Only turn on LEDs if paired
		CALL TURN_LEDS_ON
		BTFSS	IS_PAIRED, 0
		CALL	TURN_LEDS_OFF

		RETURN

;Turns on the appropriate color of the LEDs and changes TMR1 config.
TURN_LEDS_ON:
		;TEST CODE. DISABLE THE INTERRUPTS FOR NOW
		;banksel		PIE1		; Select the bank with the INTCON register
		;bcf		PIE1, TMR1IE	; Disable the timer 1 interrupt

		; Set up timer 1 with the required LED update time
		banksel		T1CON		; Select the bank with the T1CON register
		bcf		T1CON, TMR1ON	; Turn timer 1 off
		BCF T1CON, TMR1CS1 ; Set the source to instruction clock
		BCF T1CON, TMR1CS0 
		BCF T1CON, T1CKPS1 ; Set the prescaler to 2
		BSF T1CON, T1CKPS0 
		BCF T1GCON, TMR1GE ; Disable the gate control
		BSF	T1CON, TMR1ON	;Reenable the timer
		;Set the counter to 45
		MOVLW		MAX_COUNTS
		MOVWF		COUNTS_LEFT

		;Set up the LEDs
		MOVLW		0x0C
		MOVWF		LED_Timeval
		MOVWF		LED_Counter

	;		;Transfer the color into the PWM color field
	;		BCF			PWM_COLOR, 0
	;		BTFSC		COLOR, 0
	;		BSF			PWM_COLOR, 0
		;Turn on the LED PWM by setting the timer load to 255, clearing the interrupt, enabling it and turning the timer on.
		banksel TMR2
		MOVLW	0xFF
		MOVWF	TMR2
		banksel	PIR1
		BCF		PIR1, TMR2IF
		banksel	PIE1
		BSF		PIE1, TMR2IE
		banksel	T2CON
		BSF		T2CON, TMR2ON
		;banksel LATA
		;BSF		LATA, LATA2
		RETURN

;This dims the LEDs
DIM_LEDS:
		CALL LED_ON_MAX
		DECFSZ LED_Counter
		GOTO CONTINUE_LEDS
		CALL RESET_TIME

CONTINUE_LEDS:		
									;decrement the counter
		;If the counter is 0, turn the LEDs off
		DECFSZ		COUNTS_LEFT, F
		RETURN

TURN_LEDS_OFF:
		;Turn off the PWM
		banksel LATA
		BCF		LATA, BLUE_LED
		BCF		LATA, RED_LED
		;Turn off timer 1 and disable its interrupt
		BCF PIE1, TMR1IE ; Clear the timer 1 interrupt enable
		banksel T1CON
		BCF T1CON, TMR1ON ; Turn off timer 1

		;Turn off the PWM timer, disable its interrupt and clear its flag
		banksel T2CON
		BCF		T2CON, TMR2ON
		banksel PIE1
		BCF		PIE1, TMR2IE
		banksel	PIR1
		BCF		PIR1, TMR2IF

		RETURN

RESET_TIME:
		decfsz LED_Timeval
		GOTO CONTINUE_RESET
		incf	LED_Timeval
		incf	LED_Timeval
CONTINUE_RESET:
		movf LED_Timeval, W
		movwf LED_Counter
		Call LED_ON_MIN
		RETURN

LED_ON_MAX:
		movlw 0x01
		movwf LED_Lowerval
		movlw 0xFF
		movwf LED_Upperval
		RETURN

LED_ON_MIN:
		movlw 0x01
		movwf LED_Upperval
		movlw 0xFF
		movwf LED_Lowerval
		RETURN
		

PWM: 
		; Turn Timer2 OFF
		banksel T2CON
		bcf T2CON, TMR2ON ; Turn Timer2 OFF
		banksel PIR1
		BCF		PIR1, TMR2IF
		BTFSC	LED_ON, 0; Test the LED_On_Flag, skip if clear
		GOTO	PWM_GO_LO; Go to LOW_TIME
		GOTO	PWM_GO_HI; Go to HIGH_TIME

PWM_GO_HI: 	; Go to the next look up table value

		movf LED_Upperval, W

			 banksel 	PR2				; Select the bank with PR2
			 movwf 		PR2				; Move 0xFF to PR2 so that the timer counts through 255 (not including pre or post scalers)
		
			banksel LATA
			btfsc COLOR,0			; Test the color flag, skip if set
			BSF LATA, BLUE_LED				; Turn on blue LED
			btfss COLOR,0			; Test the color flag, skip if clear
			BSF LATA, RED_LED 				; Turn on blue LED
			BSF LED_ON,0			; Set the on flag

			; Turn Timer2 ON
			banksel T2CON
			bsf T2CON, TMR2ON ; Turn Timer2 ON

			GOTO POP


PWM_GO_LO: 	; Go to the next look up table value
			movf LED_Lowerval, W

			 banksel 	PR2				; Select the bank with PR2
			 movwf 		PR2				; Move 0xFF to PR2 so that the timer counts through 255 (not including pre or post scalers)

			banksel LATA
			BCF		LATA, BLUE_LED
			BCF		LATA, RED_LED
			BCF 	LED_ON,0		; Clear the on flag
			; Turn Timer2 ON
			banksel T2CON
			bsf T2CON, TMR2ON ; Turn Timer2 ON

		GOTO POP
		
POP: 
	clrf STATUS ;select bank 0
	movf PCLATH_TEMP,W ;store saved PCLATH value in WREG
	movwf PCLATH ;restore PCLATH
	movf STATUS_TEMP,W ;store saved STATUS value in WREG
	movwf STATUS ;restore STATUS
	swapf WREG_TEMP,F ;prepare WREG to be restored
	swapf WREG_TEMP,W ;restore WREG keeping STATUS bits
	retfie ;return from interrupt

	END