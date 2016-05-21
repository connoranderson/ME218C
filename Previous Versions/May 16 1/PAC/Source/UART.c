
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
#include "inc/hw_uart.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "PACSM.h"
#include "UART.h"
#include "Xbee.h"

#define RECEIVE_LENGTH_LSB 2 // Location of LSB of length bytes in incoming message

// Block out memory for pointers
static uint8_t outgoingMessageArray[50];
static uint8_t incomingMessageArray[50];

static uint8_t *outgoingMessage = &outgoingMessageArray[0]; // First entry in array
static uint8_t *incomingMessage = &incomingMessageArray[0];
static uint8_t incomingMessageSize = 8; // Used for keeping track of the size of the last message received. 
																				// Don't set below 2, or else receive ISR will stop getting LSB of length bytes.
static uint8_t outgoingMessageSize;
static uint8_t txIndex = 0;
static uint8_t rxIndex = 0;

/****************************************************************************
 Function
   InitUart1

 Description
	Initialization of Rx and Tx for UART 1 communcation with the Xbee on UART1

****************************************************************************/
void InitUart1(void){ 
	
/*	UART Initialization
1. Enable the UART module using the RCGCUART register
2. Wait for the UART to be ready (PRUART)
3. Enable the clock to the appropriate GPIO module via the RCGCGPIO
4. Wait for the GPIO module to be ready (PRGPIO)
5. Configure the GPIO pins for in/out/drive-level/drive-type
6. Select the Alternate function for the UART pins (AFSEL)
7. Configure the PMCn fields in the GPIOPCTL register to assign the UART pins
8. Disable the UART by clearing the UARTEN bit in the UARTCTL register.
9. Write the integer portion of the BRD to the UARTIBRD register.
10. Write the fractional portion of the BRD to the UARTFBRD register.
11. Write the desired serial parameters to the UARTLCRH register.
12. Configure the UART operation using the UARTCTL register.
13. Enable the UART by setting the UARTEN bit in the UARTCTL register. */
	
	HWREG(SYSCTL_RCGCUART) |= BIT1HI; // Enable UART Module 1
	while ((HWREG(SYSCTL_PRUART) & SYSCTL_PRUART_R1) != SYSCTL_PRUART_R1);	// Wait until initialized
	HWREG(SYSCTL_RCGCGPIO) |= BIT2HI; // Enable clock to GPIO C module
	while ((HWREG(SYSCTL_PRGPIO) & SYSCTL_PRGPIO_R2) != SYSCTL_PRGPIO_R2);	// Wait until initialized
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (GPIO_PIN_4);	// Initialize C4 as INPUT (receive)
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) &= (BIT4LO);
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (GPIO_PIN_5);	// Initialize C5 as OUTPUT (transmit)
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= (BIT5HI);
	
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL) |= (BIT4HI | BIT5HI); // Enable alternate functions on these GPIO pins
	HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) |= 2<<(4*4); //Put a 1 in PCTL in position PMC4 and PMC5 to select UART1 for GPIO pins 4 and 5
	HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) |= 2<<(4*5);
	HWREG(UART1_BASE + UART_O_CTL) &= BIT0LO; // Turn UART OFF

	// Baud Rate = 9600, Integer = 260, Fraction = 27
	HWREG(UART1_BASE + UART_O_IBRD) = 0x0104; 
	HWREG(UART1_BASE + UART_O_FBRD) = 0x1B; 
	HWREG(UART1_BASE + UART_O_LCRH) |= (BIT5HI|BIT6HI); // 8 bit word length. Can be updated depending on Parity settings and word length
	HWREG(UART1_BASE + UART_O_CTL) |= (BIT9HI | BIT8HI); // Enables transmit and receive on UART
	HWREG(UART1_BASE + UART_O_CTL) |= BIT0HI; // Turn UART ON	
	
	// Make sure interrupts are enabled for UART
	HWREG(UART1_BASE + UART_O_IM) |= (UART_IM_RXIM); // Local interrupt Rx enable
	HWREG(NVIC_EN0) |= BIT6HI;

	// Make sure interrupts are enabled globally
	__enable_irq( );
	
}

/****************************************************************************
 Function
   SendMessage

 Description
	Outputs an 8 bit message to the UART

****************************************************************************/
bool SendMessage(uint8_t data){ 
	// Check if we have room in FIFO
	if((HWREG(UART1_BASE + UART_O_FR)&UART_FR_TXFE) != 0){
		HWREG(UART1_BASE + UART_O_DR) = data; 
	}else{
		printf("There's no room in outgoing register. Data hasn't been transferred yet.");
	}	
	return true;
}


/****************************************************************************
 Function
   UART1_ISR

 Description
	 ISR for Interrupts Tx and Rx

****************************************************************************/
void UART1_ISR( void){ 
	//printf("\r\nInterrupt Received");
	// Check if interrupt came from TXMIS
	if((HWREG(UART1_BASE + UART_O_MIS)&UART_MIS_TXMIS) != 0){
		// Clear source of interrupt
		HWREG(UART1_BASE + UART_O_MIS) &= ~UART_MIS_TXMIS;
		// Check if we have room to transmit 
		if((HWREG(UART1_BASE + UART_O_FR)&UART_FR_TXFE) != 0){
			// if not last message
			if(txIndex < outgoingMessageSize){
				uint8_t nextMessage  = *(outgoingMessage+txIndex);	
				//printf("   nextMessage %i  ",nextMessage);
				SendMessage(nextMessage);
				txIndex++;
			}else{
				//printf("\r\n End of Transmission");
				// disable TXIM
				HWREG(UART1_BASE + UART_O_IM) &= ~UART_IM_TXIM; // Disable transmit interrupts 
				// Reset counter
				txIndex = 0;				
			}						
		}else{
			printf("There's no room in outgoing register. Data hasn't been transferred yet.");
		}
	}
	
	// Check if interrupt came from RXMIS
	if((HWREG(UART1_BASE + UART_O_MIS)&UART_MIS_RXMIS) != 0){
		// Clear source of interrupt
		HWREG(UART1_BASE + UART_O_MIS) &= ~UART_MIS_RXMIS;
		uint8_t incomingByte = HWREG(UART1_BASE + UART_O_DR);
		// Isolate error bits with a mask
		uint8_t isolatedErrorBits = incomingByte & (BIT9HI | BIT10HI | BIT11HI);
		// Check if incoming data has error
		if(isolatedErrorBits != 0){
			printf("Incoming data had an error");			
		}else{   
		// If we're not at the end of the message      
			if(rxIndex < incomingMessageSize - 1) { 
				// Save byte
				*(incomingMessage + rxIndex) = incomingByte; // Put new byte in LSB
//				printf("\r\niM : %d", *(incomingMessage + rxIndex)); // FOR DEBUGGING
				// Set the size of the incoming message
				if(rxIndex == RECEIVE_LENGTH_LSB){ // Check if we're looking at LSB bit of incoming data length frame
					incomingMessageSize = incomingByte + 4; // Set the size of the incoming message to receive all bytes. +4 to include start
																									// delimiter, 2 length bytes, and the checksum	
				}
				// Increment index
				rxIndex++;
			} else{ 
				// Handle final byte & reset index (needs to occur in this interrupt, since we will stop receiving interrupts after this point)
				if(rxIndex == (incomingMessageSize - 1)){    // Handles the final bit to make sure index gets reset.
					// Save final byte
					*(incomingMessage + rxIndex) = incomingByte; // Put new byte in LSB
//					printf("\r\niM : %d", *(incomingMessage + rxIndex));// FOR DEBUGGING
					// Reset index counter
					rxIndex = 0;
					//printf("\r\nMessage Received");
					uint8_t status = CheckForErrors(incomingMessage, incomingMessageSize);
					ES_Event ThisEvent;
					ThisEvent.EventType = MESSAGE_RECEIVED;
					ThisEvent.EventParam = status;
					PostPACSM(ThisEvent);
					printf("\r\nStatus of message: %d",status);
				} 
				
			}
		}
		
	
	}
}

/****************************************************************************
 Function
   BeginXbeeTransmission

 Description
	 Enables transmit interrupts and sends first packet (rest are handled by ISR)

****************************************************************************/
bool BeginMessageTransmission( void){ 
	//puts("Beginning Transmission");
	// Check if we have room to transmit 
	if((HWREG(UART1_BASE + UART_O_FR)&UART_FR_TXFE) != 0){
			SendMessage(*outgoingMessage);
			HWREG(UART1_BASE + UART_O_IM) |= UART_IM_TXIM; // Enable transmit interrupts 
			txIndex++;
	}else{
		printf("There's no room in outgoing register. Data hasn't been transferred yet.");
	}	
	return true;
}

/****************************************************************************
 Function
   LoadData

 Description
	 Prepares data for next transmission (!should be called BEFORE BeginMessageTransmission!)

****************************************************************************/
void LoadData(uint8_t *transmitData, uint8_t SizeOfArray){ 
	outgoingMessage = transmitData;
	outgoingMessageSize = SizeOfArray;
}

/****************************************************************************
 Function
   GetMessage

 Description
	 Returns a pointer to the first index of the incoming message

****************************************************************************/
uint8_t * GetMessage( void){ 
	return incomingMessage;
}

/****************************************************************************
 Function
   GetMessageSize

 Description
	 Returns the size of the whole received message

****************************************************************************/
uint8_t GetMessageSize( void){ 
	return incomingMessageSize;
}

/****************************************************************************
 Function
   SendTestMessage

 Description
	Outputs a message to test the functionality of the UART

****************************************************************************/
bool SendTestMessage(void){ 
	// Check if we have room to transmit 
	if((HWREG(UART1_BASE + UART_O_FR)&UART_FR_TXFE) != 0){
		HWREG(UART1_BASE + UART_O_DR) = (BIT0HI|BIT2HI|BIT4HI|BIT6HI); // Random test packet
	}else{
		printf("There's no room in outgoing register. Data hasn't been transferred yet.");
	}	
	return true;
}



