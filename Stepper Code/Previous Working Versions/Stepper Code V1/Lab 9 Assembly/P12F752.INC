        LIST

;==========================================================================
;  MPASM PIC12F752 processor include
; 
;  (c) Copyright 1999-2010 Microchip Technology, All rights reserved
;==========================================================================

        NOLIST

;==========================================================================
;  This header file defines configurations, registers, and other useful
;  bits of information for the PIC12F752 microcontroller.  These names
;  are taken to match the data sheets as closely as possible.
;
;  Note that the processor must be selected before this file is included.
;  The processor may be selected the following ways:
;
;       1. Command line switch:
;               C:\MPASM MYFILE.ASM /PIC12F752
;       2. LIST directive in the source file
;               LIST   P=PIC12F752
;       3. Processor Type entry in the MPASM full-screen interface
;       4. Setting the processor in the MPLAB Project Dialog
;==========================================================================

;==========================================================================
;
;       Verify Processor
;
;==========================================================================
        IFNDEF __12F752
           MESSG "Processor-header file mismatch.  Verify selected processor."
        ENDIF



;==========================================================================
;
;       Register Definitions
;
;==========================================================================

W                EQU  H'0000'
F                EQU  H'0001'

;----- Register Files -----------------------------------------------------

;-----Bank0------------------
INDF             EQU  H'0000'
TMR0             EQU  H'0001'
PCL              EQU  H'0002'
STATUS           EQU  H'0003'
FSR              EQU  H'0004'
PORTA            EQU  H'0005'
IOCAF            EQU  H'0008'
PCLATH           EQU  H'000A'
INTCON           EQU  H'000B'
PIR1             EQU  H'000C'
PIR2             EQU  H'000D'
PIR3             EQU  H'000E'
TMR1L            EQU  H'000F'
TMR1H            EQU  H'0010'
T1CON            EQU  H'0011'
T1GCON           EQU  H'0012'
CCPR1            EQU  H'0013'
CCPR1L           EQU  H'0013'
CCPR1H           EQU  H'0014'
CCP1CON          EQU  H'0015'
ADRES            EQU  H'001C'
ADRESL           EQU  H'001C'
ADRESH           EQU  H'001D'
ADCON0           EQU  H'001E'
ADCON1           EQU  H'001F'

;-----Bank1------------------
OPTION_REG       EQU  H'0081'
TRISA            EQU  H'0085'
IOCAP            EQU  H'0088'
PIE1             EQU  H'008C'
PIE2             EQU  H'008D'
PIE3             EQU  H'008E'
OSCCON           EQU  H'008F'
FVRCON           EQU  H'0090'
DACCON0          EQU  H'0091'
DACCON1          EQU  H'0092'
C2CON0           EQU  H'009B'
CM2CON0          EQU  H'009B'
C2CON1           EQU  H'009C'
CM2CON1          EQU  H'009C'
C1CON0           EQU  H'009D'
CM1CON0          EQU  H'009D'
C1CON1           EQU  H'009E'
CM1CON1          EQU  H'009E'
CMOUT            EQU  H'009F'
MCOUT            EQU  H'009F'

;-----Bank2------------------
LATA             EQU  H'0105'
IOCAN            EQU  H'0108'
WPUA             EQU  H'010C'
PCON             EQU  H'010F'
TMR2             EQU  H'0110'
PR2              EQU  H'0111'
T2CON            EQU  H'0112'
HLTTMR           EQU  H'0113'
HLTPR            EQU  H'0114'
HLTCON0          EQU  H'0115'
HLTCON1          EQU  H'0116'

;-----Bank3------------------
ANSELA           EQU  H'0185'
APFCON           EQU  H'0188'
OSCTUNE          EQU  H'0189'
PMCON1           EQU  H'018C'
PMCON2           EQU  H'018D'
PMADR            EQU  H'018E'
PMADRL           EQU  H'018E'
PMADRH           EQU  H'018F'
PMDAT            EQU  H'0190'
PMDATL           EQU  H'0190'
PMDATH           EQU  H'0191'
CWGPH            EQU  H'0192'
CWGBKR           EQU  H'0193'
CWGDBR           EQU  H'0194'
CWGCON0          EQU  H'0195'
CWGCON1          EQU  H'0196'
CWGCON2          EQU  H'0197'
CWGASD           EQU  H'0198'

;----- STATUS Bits -----------------------------------------------------
C                EQU  H'0000'
DC               EQU  H'0001'
Z                EQU  H'0002'
NOT_PD           EQU  H'0003'
NOT_TO           EQU  H'0004'
IRP              EQU  H'0007'


;----- PORTA Bits -----------------------------------------------------
RA0              EQU  H'0000'
RA1              EQU  H'0001'
RA2              EQU  H'0002'
RA3              EQU  H'0003'
RA4              EQU  H'0004'
RA5              EQU  H'0005'


;----- IOCAF Bits -----------------------------------------------------
IOCAF0           EQU  H'0000'
IOCAF1           EQU  H'0001'
IOCAF2           EQU  H'0002'
IOCAF3           EQU  H'0003'
IOCAF4           EQU  H'0004'
IOCAF5           EQU  H'0005'


;----- INTCON Bits -----------------------------------------------------
RAIF             EQU  H'0000'
INTF             EQU  H'0001'
TMR0IF           EQU  H'0002'
GPIE             EQU  H'0003'
INTE             EQU  H'0004'
TMR0IE           EQU  H'0005'
PEIE             EQU  H'0006'
GIE              EQU  H'0007'


;----- PIR1 Bits -----------------------------------------------------
TMR1IF           EQU  H'0000'
TMR2IF           EQU  H'0001'
HLTIF            EQU  H'0002'
ADIF             EQU  H'0006'
TMR1GIF          EQU  H'0007'


;----- PIR2 Bits -----------------------------------------------------
CCP1IF           EQU  H'0000'
C1IF             EQU  H'0004'
C2IF             EQU  H'0005'


;----- PIR3 Bits -----------------------------------------------------
CWG1IF           EQU  H'0000'


;----- T1CON Bits -----------------------------------------------------
TMR1ON           EQU  H'0000'
NOT_T1SYNC       EQU  H'0002'
T1OSCEN          EQU  H'0003'
T1CKPS0          EQU  H'0004'
T1CKPS1          EQU  H'0005'
TMR1CS0          EQU  H'0006'
TMR1CS1          EQU  H'0007'



;----- T1GCON Bits -----------------------------------------------------
T1GSS0           EQU  H'0000'
T1GSS1           EQU  H'0001'
T1GVAL           EQU  H'0002'
T1GGO_NOT_DONE   EQU  H'0003'
T1GSPM           EQU  H'0004'
T1GTM            EQU  H'0005'
T1GPOL           EQU  H'0006'
TMR1GE           EQU  H'0007'

T1GGO            EQU  H'0003'


;----- CCP1CON Bits -----------------------------------------------------
CCPM0            EQU  H'0000'
CCPM1            EQU  H'0001'
CCPM2            EQU  H'0002'
CCPM3            EQU  H'0003'
DCB0             EQU  H'0004'
DCB1             EQU  H'0005'


;----- ADCON0 Bits -----------------------------------------------------
ADON             EQU  H'0000'
GO_NOT_DONE      EQU  H'0001'
CHS0             EQU  H'0002'
CHS1             EQU  H'0003'
CHS2             EQU  H'0004'
CHS3             EQU  H'0005'
VCFG             EQU  H'0006'
ADFM             EQU  H'0007'


GO               EQU  H'0001'

GO_DONE          EQU  H'0001'

NOT_DONE         EQU  H'0001'

ADGO             EQU  H'0001'


;----- ADCON1 Bits -----------------------------------------------------
ADCS0            EQU  H'0004'
ADCS1            EQU  H'0005'
ADCS2            EQU  H'0006'



;----- OPTION_REG Bits -----------------------------------------------------
PS0              EQU  H'0000'
PS1              EQU  H'0001'
PS2              EQU  H'0002'
PSA              EQU  H'0003'
T0SE             EQU  H'0004'
T0CS             EQU  H'0005'
INTEDG           EQU  H'0006'
NOT_GPPU         EQU  H'0007'


;----- TRISA Bits -----------------------------------------------------
TRISA0           EQU  H'0000'
TRISA1           EQU  H'0001'
TRISA2           EQU  H'0002'
TRISA3           EQU  H'0003'
TRISA4           EQU  H'0004'
TRISA5           EQU  H'0005'


;----- IOCAP Bits -----------------------------------------------------
IOCAP0           EQU  H'0000'
IOCAP1           EQU  H'0001'
IOCAP2           EQU  H'0002'
IOCAP3           EQU  H'0003'
IOCAP4           EQU  H'0004'
IOCAP5           EQU  H'0005'


;----- PIE1 Bits -----------------------------------------------------
TMR1IE           EQU  H'0000'
TMR2IE           EQU  H'0001'
HLTIE            EQU  H'0002'
ADIE             EQU  H'0006'
TMR1GIE          EQU  H'0007'


;----- PIE2 Bits -----------------------------------------------------
CCP1IE           EQU  H'0000'
C1IE             EQU  H'0004'
C2IE             EQU  H'0005'


;----- PIE3 Bits -----------------------------------------------------
CWGIE            EQU  H'0000'


;----- OSCCON Bits -----------------------------------------------------
LTS              EQU  H'0001'
HTS              EQU  H'0002'
IRCF0            EQU  H'0004'
IRCF1            EQU  H'0005'


;----- FVRCON Bits -----------------------------------------------------
FVRBUFSS         EQU  H'0004'
FVRBUFEN         EQU  H'0005'
FVRRDY           EQU  H'0006'
FVREN            EQU  H'0007'


;----- DACCON0 Bits -----------------------------------------------------
DACPSS0          EQU  H'0002'
DACOE            EQU  H'0005'
DACRNG           EQU  H'0006'
DACEN            EQU  H'0007'


;----- DACCON1 Bits -----------------------------------------------------
DACR0            EQU  H'0000'
DACR1            EQU  H'0001'
DACR2            EQU  H'0002'
DACR3            EQU  H'0003'
DACR4            EQU  H'0004'



;----- C2CON0 Bits -----------------------------------------------------
C2SYNC           EQU  H'0000'
C2HYS            EQU  H'0001'
C2SP             EQU  H'0002'
C2POL            EQU  H'0004'
C2OE             EQU  H'0005'
C2OUT            EQU  H'0006'
C2ON             EQU  H'0007'


;----- CM2CON0 Bits -----------------------------------------------------
C2SYNC           EQU  H'0000'
C2HYS            EQU  H'0001'
C2SP             EQU  H'0002'
C2POL            EQU  H'0004'
C2OE             EQU  H'0005'
C2OUT            EQU  H'0006'
C2ON             EQU  H'0007'


;----- C2CON1 Bits -----------------------------------------------------
C2NCH            EQU  H'0000'
C2PCH0           EQU  H'0004'
C2PCH1           EQU  H'0005'
C2INTN           EQU  H'0006'
C2INTP           EQU  H'0007'


;----- CM2CON1 Bits -----------------------------------------------------
C2NCH            EQU  H'0000'
C2PCH0           EQU  H'0004'
C2PCH1           EQU  H'0005'
C2INTN           EQU  H'0006'
C2INTP           EQU  H'0007'


;----- C1CON0 Bits -----------------------------------------------------
C1SYNC           EQU  H'0000'
C1HYS            EQU  H'0001'
C1SP             EQU  H'0002'
C1POL            EQU  H'0004'
C1OE             EQU  H'0005'
C1OUT            EQU  H'0006'
C1ON             EQU  H'0007'


;----- CM1CON0 Bits -----------------------------------------------------
C1SYNC           EQU  H'0000'
C1HYS            EQU  H'0001'
C1SP             EQU  H'0002'
C1POL            EQU  H'0004'
C1OE             EQU  H'0005'
C1OUT            EQU  H'0006'
C1ON             EQU  H'0007'


;----- C1CON1 Bits -----------------------------------------------------
C1NCH            EQU  H'0000'
C1PCH0           EQU  H'0004'
C1PCH1           EQU  H'0005'
C1INTN           EQU  H'0006'
C1INTP           EQU  H'0007'


;----- CM1CON1 Bits -----------------------------------------------------
C1NCH            EQU  H'0000'
C1PCH0           EQU  H'0004'
C1PCH1           EQU  H'0005'
C1INTN           EQU  H'0006'
C1INTP           EQU  H'0007'


;----- CMOUT Bits -----------------------------------------------------
MCOUT1           EQU  H'0000'
MCOUT2           EQU  H'0001'


;----- MCOUT Bits -----------------------------------------------------
MCOUT1           EQU  H'0000'
MCOUT2           EQU  H'0001'


;----- LATA Bits -----------------------------------------------------
LATA0            EQU  H'0000'
LATA1            EQU  H'0001'
LATA2            EQU  H'0002'
LATA4            EQU  H'0004'
LATA5            EQU  H'0005'


;----- IOCAN Bits -----------------------------------------------------
IOCAN0           EQU  H'0000'
IOCAN1           EQU  H'0001'
IOCAN3           EQU  H'0002'
IOCAN4           EQU  H'0003'
IOCAN5           EQU  H'0004'


;----- WPUA Bits -----------------------------------------------------
WPUA0            EQU  H'0000'
WPUA1            EQU  H'0001'
WPUA2            EQU  H'0002'
WPUA3            EQU  H'0003'
WUA4             EQU  H'0004'
WPUA5            EQU  H'0005'


;----- PCON Bits -----------------------------------------------------
NOT_BOR          EQU  H'0000'
NOT_POR          EQU  H'0001'


;----- T2CON Bits -----------------------------------------------------
T2CKPS0          EQU  H'0000'
T2CKPS1          EQU  H'0001'
TMR2ON           EQU  H'0002'
T2OUTPS0         EQU  H'0003'
T2OUTPS1         EQU  H'0004'
T2OUTPS2         EQU  H'0005'
T2OUTPS3         EQU  H'0006'



;----- HLTCON0 Bits -----------------------------------------------------
HLTCKPS0         EQU  H'0000'
HLTCKPS1         EQU  H'0001'
HLTON            EQU  H'0002'
HLTOUTPS0        EQU  H'0003'
HLTOUTPS1        EQU  H'0004'
HLTOUTPS2        EQU  H'0005'
HLTOUTPS3        EQU  H'0006'



;----- HLTCON1 Bits -----------------------------------------------------
HLTREREN         EQU  H'0000'
HLTFEREN         EQU  H'0001'
HLTERS0          EQU  H'0002'
HLTERS1          EQU  H'0003'
HLTERS2          EQU  H'0004'



;----- ANSELA Bits -----------------------------------------------------
ANSA0            EQU  H'0000'
ANSA1            EQU  H'0001'
ANSA2            EQU  H'0002'
ANSA4            EQU  H'0004'
ANSA5            EQU  H'0005'


;----- APFCON Bits -----------------------------------------------------
CWGO0SEL         EQU  H'0000'
CWGO1SEL         EQU  H'0001'
CWGFSEL          EQU  H'0002'
T1GSEL           EQU  H'0004'


;----- OSCTUNE Bits -----------------------------------------------------
TUN0             EQU  H'0000'
TUN1             EQU  H'0001'
TUN2             EQU  H'0002'
TUN3             EQU  H'0003'
TUN4             EQU  H'0004'



;----- PMCON1 Bits -----------------------------------------------------
RD               EQU  H'0000'
WR               EQU  H'0001'
WREN             EQU  H'0002'


;----- CWGPH Bits -----------------------------------------------------
PH0              EQU  H'0000'
PH1              EQU  H'0001'
PH2              EQU  H'0002'
PH3              EQU  H'0003'



;----- CWGBKR Bits -----------------------------------------------------
BKF0             EQU  H'0000'
BKF1             EQU  H'0001'
BKF2             EQU  H'0002'
BKF3             EQU  H'0003'
BKR0             EQU  H'0004'
BKR1             EQU  H'0005'
BKR2             EQU  H'0006'
BKR3             EQU  H'0007'



;----- CWGDBR Bits -----------------------------------------------------
DBF0             EQU  H'0000'
DBF1             EQU  H'0001'
DBF2             EQU  H'0002'
DBF3             EQU  H'0003'
DBR0             EQU  H'0004'
DBR1             EQU  H'0005'
DBR2             EQU  H'0006'
DBR3             EQU  H'0007'



;----- CWGCON0 Bits -----------------------------------------------------
CWGCS0           EQU  H'0001'
CWGCS1           EQU  H'0002'
CWGPOL1          EQU  H'0003'
CWGPOL0          EQU  H'0004'
CWGOE1           EQU  H'0005'
CWGOE0           EQU  H'0006'
CWGEN            EQU  H'0007'



;----- CWGCON1 Bits -----------------------------------------------------
CWGRS0           EQU  H'0000'
CWGRS1           EQU  H'0001'
CWGRS2           EQU  H'0002'
CWGFS0           EQU  H'0003'
CWGFS1           EQU  H'0004'
CWGFS2           EQU  H'0005'
CWGLD            EQU  H'0007'



;----- CWGCON2 Bits -----------------------------------------------------
CWGRBK0          EQU  H'0000'
CWGRBK1          EQU  H'0001'
CWGRBK2          EQU  H'0002'
CWGFBK0          EQU  H'0003'
CWGFBK1          EQU  H'0004'
CWGFBK2          EQU  H'0005'
CWGFDB           EQU  H'0006'
CWGRDB           EQU  H'0007'


;----- CWGASD Bits -----------------------------------------------------
CWGAS0E          EQU  H'0000'
CWGAS1E          EQU  H'0001'
CWGAS2E          EQU  H'0002'
CWGAS3E          EQU  H'0003'
CWGAOD0          EQU  H'0004'
CWGAOD1          EQU  H'0005'
CWGARSE          EQU  H'0006'
CWGASDE          EQU  H'0007'



;==========================================================================
;
;       RAM Definitions
;
;==========================================================================
       __MAXRAM  H'01FF'
       __BADRAM  H'0006'-H'0007'
       __BADRAM  H'0009'
       __BADRAM  H'0016'-H'001B'
       __BADRAM  H'0020'-H'003F'
       __BADRAM  H'0086'-H'0087'
       __BADRAM  H'0089'
       __BADRAM  H'0093'-H'009A'
       __BADRAM  H'00A0'-H'00EF'
       __BADRAM  H'0106'-H'0107'
       __BADRAM  H'0109'
       __BADRAM  H'010D'-H'010E'
       __BADRAM  H'0117'-H'011F'
       __BADRAM  H'0120'-H'016F'
       __BADRAM  H'0186'-H'0187'
       __BADRAM  H'0199'-H'019F'
       __BADRAM  H'01A0'-H'01EF'

;==========================================================================
;
;       Configuration Bits
;
;   NAME            Address
;   CONFIG            2007h
;
;==========================================================================

; The following is an assignment of address values for all of the
; configuration registers for the purpose of table reads
_CONFIG          EQU  H'2007'

;----- CONFIG Options --------------------------------------------------
_FOSC_INT            EQU  H'3FFE'    ; Internal oscillator mode.  I/O function on RA5/CLKIN
_FOSC_EC             EQU  H'3FFF'    ; EC oscillator mode.  CLKIN function on RA5/CLKIN

_WDT_OFF             EQU  H'3FF7'    ; Watchdog Timer disabled
_WDT_ON              EQU  H'3FFF'    ; Watchdog Timer enabled

_PWRTE_ON            EQU  H'3FEF'    ; Powerup Timer enabled
_PWRTE_OFF           EQU  H'3FFF'    ; Powerup Timer disabled

_MCLRE_OFF           EQU  H'3FDF'    ; MCLR pin is alternate function
_MCLRE_ON            EQU  H'3FFF'    ; MCLR pin is MCLR function with internal weak pullup

_CP_ON               EQU  H'3FBF'    ; Code protection enabled
_CP_OFF              EQU  H'3FFF'    ; Code protection disabled

_BOREN_DIS           EQU  H'3CFF'    ; BOR disabled
_BOREN_SLEEP_DIS     EQU  H'3EFF'    ; BOR disabled during Sleep and enabled during run
_BOREN_EN            EQU  H'3FFF'    ; BOR enabled

_WRT_ALL             EQU  H'33FF'    ; 000h to 3FFh write protected
_WRT_HALF            EQU  H'37FF'    ; 000h to 1FFh write protected
_WRT_FOURTH          EQU  H'3BFF'    ; 000h to 0FFh write protected
_WRT_OFF             EQU  H'3FFF'    ; Flash write protection off

_CKLOUTEN_ON         EQU  H'2FFF'    ; CLKOUT function enabled.  CLKOUT pin is CLKOUT
_CKLOUTEN_OFF        EQU  H'3FFF'    ; CLKOUT function disabled.  CLKOUT pin acts as I/O

;----- DEVID Equates --------------------------------------------------
_DEVID1          EQU  H'2006'

;----- IDLOC Equates --------------------------------------------------
_IDLOC0          EQU  H'2000'
_IDLOC1          EQU  H'2001'
_IDLOC2          EQU  H'2002'
_IDLOC3          EQU  H'2003'

        LIST
