// TI File $Revision: /main/2 $
// Checkin $Date: July 31, 2009   17:59:15 $
//###########################################################################
//
// FILE:   Example_281xSpi_FFDLB_int.c
//
// TITLE:  DSP281x Device Spi Digital Loop Back porgram.
//
// ASSUMPTIONS:
//
//         This program requires the DSP281x V1.00 header files.
//         As supplied, this project is configured for "boot to H0" operation.
//
//         Other then boot mode pin configuration, no other hardware configuration
//         is required.
//
// DESCRIPTION:
//
// This program is a SPI example that uses the internal loopback of
// the peripheral.  Both interrupts and the SPI FIFOs are used.
//
// A stream of data is sent and then compared to the recieved stream.
//
// The sent data looks like this:
// 0000 0001 0002 0003 0004 0005 0006 0007
// 0001 0002 0003 0004 0005 0006 0007 0008
// 0002 0003 0004 0005 0006 0007 0008 0009
// ....
// FFFE FFFF 0000 0001 0002 0003 0004 0005
// FFFF 0000 0001 0002 0003 0004 0005 0006
// etc..
//
// This pattern is repeated forever.
//
//
// Watch Variables:
//     sdata[8]    - Data to send
//     rdata[8]    - Received data
//     rdata_point - Used to keep track of the last position in
//                   the receive stream for error checking
//###########################################################################
// $TI Release: DSP281x C/C++ Header Files V1.20 $
// $Release Date: July 27, 2009 $
//###########################################################################

#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File

// Prototype statements for functions found within this file.
// interrupt void ISRTimer2(void);
interrupt void spiTxFifoIsr(void);
interrupt void spiRxFifoIsr(void);
void delay_loop(void);
void spi_fifo_init(void);
void error();

Uint16 sdata[8];     // Send data buffer
Uint16 rdata[8];     // Receive data buffer
Uint16 rdata_point;  // Keep track of where we are
                     // in the data stream to check received data

void main(void)
{
   Uint16 i;

// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP281x_SysCtrl.c file.
   InitSysCtrl();

// Step 2. Initalize GPIO:
// This example function is found in the DSP281x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
// InitGpio();  // Skipped for this example
// Setup only the GP I/O only for SPI functionality
   EALLOW;
   GpioMuxRegs.GPFMUX.all=0x000F;	// Select GPIOs to be SPI pins
                                    // Port F MUX - x000 0000 0000 1111
   EDIS;

// Step 3. Initialize PIE vector table:
// Disable and clear all CPU interrupts
   DINT;
   IER = 0x0000;
   IFR = 0x0000;

// Initialize PIE control registers to their default state:
// This function is found in the DSP281x_PieCtrl.c file.
   InitPieCtrl();

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP281x_DefaultIsr.c.
// This function is found in DSP281x_PieVect.c.
   InitPieVectTable();

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
   EALLOW;	// This is needed to write to EALLOW protected registers
   PieVectTable.SPIRXINTA = &spiRxFifoIsr;
   PieVectTable.SPITXINTA = &spiTxFifoIsr;
   EDIS;   // This is needed to disable write to EALLOW protected registers


// Step 4. Initialize all the Device Peripherals:
// This function is found in DSP281x_InitPeripherals.c
// InitPeripherals(); // Not required for this example
   spi_fifo_init();	  // Initialize the SPI only


// Step 5. User specific code, enable interrupts:

// Initalize the send data buffer
   for(i=0; i<8; i++)
   {
      sdata[i] = i;
   }
   rdata_point = 0;

// Enable interrupts required for this example
   PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   // Enable the PIE block
   PieCtrlRegs.PIEIER6.bit.INTx1=1;     // Enable PIE Group 6, INT 1
   PieCtrlRegs.PIEIER6.bit.INTx2=1;     // Enable PIE Group 6, INT 2
   IER=0x20;                            // Enable CPU INT6
   EINT;                                // Enable Global Interrupts

// Step 6. IDLE loop. Just sit and loop forever (optional):
	for(;;);

}


// Some Useful local functions
void delay_loop()
{
    long      i;
    for (i = 0; i < 1000000; i++) {}
}


void error(void)
{
    asm("     ESTOP0");	 //Test failed!! Stop!
    for (;;);
}


void spi_fifo_init()
{
// Initialize SPI FIFO registers
   SpiaRegs.SPICCR.bit.SPISWRESET=0; // Reset SPI

   SpiaRegs.SPICCR.all=0x001F;       //16-bit character, Loopback mode
   SpiaRegs.SPICTL.all=0x0017;       //Interrupt enabled, Master/Slave XMIT enabled
   SpiaRegs.SPISTS.all=0x0000;
   SpiaRegs.SPIBRR=0x0063;           // Baud rate
   SpiaRegs.SPIFFTX.all=0xC028;      // Enable FIFO's, set TX FIFO level to 8
   SpiaRegs.SPIFFRX.all=0x0028;      // Set RX FIFO level to 8
   SpiaRegs.SPIFFCT.all=0x00;
   SpiaRegs.SPIPRI.all=0x0010;

   SpiaRegs.SPICCR.bit.SPISWRESET=1;  // Enable SPI

   SpiaRegs.SPIFFTX.bit.TXFIFO=1;
   SpiaRegs.SPIFFRX.bit.RXFIFORESET=1;
}

interrupt void spiTxFifoIsr(void)
{
 	Uint16 i;
    for(i=0;i<8;i++)
    {
 	   SpiaRegs.SPITXBUF=sdata[i];      // Send data
    }

    for(i=0;i<8;i++)                    // Increment data for next cycle
    {
 	   sdata[i]++;
    }


    SpiaRegs.SPIFFTX.bit.TXFFINTCLR=1;  // Clear Interrupt flag
	PieCtrlRegs.PIEACK.all|=0x20;  		// Issue PIE ACK
}

interrupt void spiRxFifoIsr(void)
{
    Uint16 i;
    for(i=0;i<8;i++)
    {
	    rdata[i]=SpiaRegs.SPIRXBUF;		// Read data
	}
	for(i=0;i<8;i++)                    // Check received data
	{
	    if(rdata[i] != rdata_point+i) error();
	}
	rdata_point++;
	SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1;  // Clear Overflow flag
	SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1; 	// Clear Interrupt flag
	PieCtrlRegs.PIEACK.all|=0x20;       // Issue PIE ack
}



//===========================================================================
// No more.
//===========================================================================

