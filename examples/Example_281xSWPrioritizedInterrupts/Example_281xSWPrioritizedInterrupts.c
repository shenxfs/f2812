// TI File $Revision: /main/2 $
// Checkin $Date: July 31, 2009   17:59:17 $
//###########################################################################
//
// FILE:   Example_281xSWPrioritizedInterrupts.c
//
// TITLE:  DSP281x Software Prioritized Interrupt Example.
//
// ASSUMPTIONS:
//
//          This program requires the DSP281x V1.00 header files.  
//          As supplied, this project is configured for "boot to H0" operation. 
//
//          Other then boot mode pin configuration, no other hardware configuration
//          is required.
//  
//
// DESCRIPTION:
//
//          For most applications, the hardware prioritization of the 
//          the PIE module is sufficient.  For applications that need custom
//          prioritization, this example illustrates an example of
//          how this can be done through software. 
//
//          For more information on F2810/12 interrupt priorities, refer to the
//          ReadMe_Example_28xISRPriorities.pdf file included with this example.
//
//          This program simulates interrupt conflicts by writing to the
//          PIEIFR registers.  This will simulate multiple interrupts coming into
//          the PIE block at the same time. 
//
//          The interrupt service routine routines are software prioritized
//          by the table found in the DSP281x_SWPrioritizedIsrLevels.h file.
//
//          1) Before compiling you must set the Global and Group interrupt priorities
//             in the DSP281x_SWPrioritizedIsrLevels.h file.
//
//          2) Compile the code, load, and run
//
//          3) At the end of each test there is a hard coded breakpoint (ESTOP0).  When code
//             stops at the breakpoint, examine the ISRTrace buffer to see the order
//             in which the ISR's completed.    All PIE interrupts will add to the
//             ISRTrace.
//
//             The ISRTrace will consist of a list of hex values as shown:
//
//                                  0x00wx    <- PIE Group w interrup x finished first
//                                  0x00yz    <- PIE Group y interrupt z finished next
//
//          4) If desired, set a new set of Global and Group interrupt priorites
//             and repeat the test to see the change.
//          
//
//          Watch Variables:
//                ISRTrace[50]           Trace of ISR's in the order they complete
//                                       After each test, examine this buffer
//                                       to determine if the ISR's completed in
//                                       the order desired. 
//
//###########################################################################
// $TI Release: DSP281x C/C++ Header Files V1.20 $
// $Release Date: July 27, 2009 $
//###########################################################################

#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File

// Define which interrupts are used in the PIE for each group.
#define ISRS_GROUP1  (M_INT1|M_INT2|M_INT4|M_INT5|M_INT6|M_INT7|M_INT8)
#define ISRS_GROUP2  (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT7)
#define ISRS_GROUP3  (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT7)
#define ISRS_GROUP4  (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT7)
#define ISRS_GROUP5  (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT7)
#define ISRS_GROUP6  (M_INT1|M_INT2|M_INT5|M_INT6)
#define ISRS_GROUP9  (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5)


// This array will be used as a trace to check the order that the
// interrupts were serviced
Uint16  ISRTrace[50];
Uint16  ISRTraceIndex;  // used to update an element in the trace buffer

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


// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts 
   DINT;

// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.  
// This function is found in the DSP281x_PieCtrl.c file.
   InitPieCtrl();
   
// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;   

// Initialize the PIE vector table with pointers to the shell Interrupt 
// Service Routines (ISR).  
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP281x_DefaultIsr.c.
// This function is found in DSP281x_PieVect.c.
   InitPieVectTable();
   
// Step 4. Initialize all the Device Peripherals:
// This function is found in DSP281x_InitPeripherals.c
// InitPeripherals(); // Not required for this example
 
// Step 5. User specific code, enable interrupts:

// CASE 1:
//     Force all group 1 interrupts at once by writing to the PIEIFR1 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;
       
       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   

       // Enable PIE group 1 interrupt 1-8
       PieCtrlRegs.PIEIER1.all = 0x00FF;

       // Make sure PIEACK for group 1 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT1;
	
       // Enable CPU INT1
       IER |= M_INT1;

       // Force all valid interrupts for Group 1
       PieCtrlRegs.PIEIFR1.all = ISRS_GROUP1;
       
       // Enable global Interrupts CPU level:
	   EINT;   // Enable Global interrupt INTM
	   
	   // Wait for all Group 1 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR1.all != 0x0000 ){}
	   
       // Stop here and check the ISRTrace to determine which order the 
       // ISR Routines completed.  The order is dependant on the priority
       // assigned in the DSP281x_SWPrioritizedIsrLevels.h file
       // 
       // The ISRTrace will contain a list of values corresponding to the
       // interrupts serviced in the order they were serviced. 
	   // For example if the ISRTrace looks like this
	   //        0x0014     ISR Group 1 interrupt 4
	   //        0x0017     ISR Group 1 interrupt 7
	   //        0x0016     ISR Group 1 interrupt 6
	   //        0x0000     end of trace
	   asm("        ESTOP0");	

// CASE 2:
//     Force all group 2 interrupts at once by writing to the PIEIFR2 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
       
       // Enable PIE group 2 interrupts 1-8 
       PieCtrlRegs.PIEIER2.all = 0x00FF;
       
       // Enable CPU INT2
       IER |= (M_INT2);

	   // Make sure PIEACK for group 2 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT2;

       // Force all valid interrupts for Group 2
       PieCtrlRegs.PIEIFR2.all = ISRS_GROUP2;  

	   // Enable Global interrupts
	   EINT;

	   
	   // Wait for all group 2 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR2.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");
	   
	   
// CASE 3:
//     Force all group 3 interrupts at once by writing to the PIEIFR3 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
              
       // Enable PIE group 3 interrupts 1-8 
       PieCtrlRegs.PIEIER3.all = 0x00FF;

	   // Make sure PIEACK for group 3 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT3;

       // Enable CPU INT3
       IER |= (M_INT3);

       // Force all valid interrupts for Group 3
       PieCtrlRegs.PIEIFR3.all = ISRS_GROUP3;  

	   // Enable Global interrupts
	   EINT;
	   
	   // Wait for all group 3 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR3.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");	   	   	   
	   
// CASE 4:
//     Force all group 4 interrupts at once by writing to the PIEIFR4 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
       
       // Enable PIE group 4 interrupts 1-8 
       PieCtrlRegs.PIEIER4.all = 0x00FF;

	   // Make sure PIEACK for group 3 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT4;
       
       // Enable CPU INT4
       IER |= (M_INT4);

       // Force all valid interrupts for Group 4
       PieCtrlRegs.PIEIFR4.all = ISRS_GROUP4; 

	   // Enable Global interrupts
	   EINT;

	   // Wait for all group 4 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR4.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");	
	   
// CASE 5:
//     Force all group 5 interrupts at once by writing to the PIEIFR5 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
              
       // Enable PIE group 5 interrupts 1-8 
       PieCtrlRegs.PIEIER5.all = 0x00FF;

	   // Make sure PIEACK for group 5 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT5;

       // Enable CPU INT5
       IER |= (M_INT5);

       // Force all valid interrupts for Group 5
       PieCtrlRegs.PIEIFR5.all = ISRS_GROUP5;  

	   // Enable Global interrupts
	   EINT;
	   
	   // Wait for all group 5 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR5.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");
	   
// CASE 6:
//     Force all group 6 interrupts at once by writing to the PIEIFR6 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
              
       // Enable PIE group 6 interrupts 1-8 
       PieCtrlRegs.PIEIER6.all = 0x00FF;

	   // Make sure PIEACK for group 6 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT6;	   
              
       // Enable CPU INT6
       IER |= (M_INT6);

       // Force all valid interrupts for Group 6
       PieCtrlRegs.PIEIFR6.all = ISRS_GROUP6;  

	   // Enable Global interrupts
	   EINT;
	   
	   
	   // Wait for all group 6 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR6.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");

// CASE 8:
//     Force all group 9 interrupts at once by writing to the PIEIFR4 register

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;    

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
       
       // Enable PIE group 9 interrupts 1-8 
       PieCtrlRegs.PIEIER9.all = 0x00FF;
       
	   // Make sure PIEACK for group 9 is clear (default after reset)
       PieCtrlRegs.PIEACK.all = M_INT9;

       // Enable CPU INT9
       IER |= (M_INT9);

       // Force all valid interrupts for Group 9
       PieCtrlRegs.PIEIFR9.all = ISRS_GROUP9;  

	   // Enable Global interrupts
	   EINT;

	   // Wait for all group 9 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR9.all != 0x0000 ){}
	   
       // Stop here and check the order the ISR's were serviced in the
       // ISRTrace
	   asm("        ESTOP0");	
	   	
// CASE 9:
//     Force all group 1 and group 2 interrupts at once

       // Setup next test - fire interrupts from Group 1 and Group 2

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   

       // Enable PIE group 1 and group 2 interrupts 1-8 
       PieCtrlRegs.PIEIER1.all = 0x00FF;
       PieCtrlRegs.PIEIER2.all = 0x00FF;

	   // Make sure PIEACK for group 1 & 2 are clear (default after reset)
       PieCtrlRegs.PIEACK.all = (M_INT3 | M_INT2);

       // Enable CPU INT1 and INT2
       IER |= (M_INT1|M_INT2);
              
       // Force all valid interrupts for Group 1 and from Group 2
       PieCtrlRegs.PIEIFR1.all = ISRS_GROUP1;  
       PieCtrlRegs.PIEIFR2.all = ISRS_GROUP2;  

	   // Enable Global interrupts
	   EINT;
	   
	   // Wait for all group 1 and group 2 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR1.all != 0x0000 
	      || PieCtrlRegs.PIEIFR2.all != 0x0000 ){}
	   
	   // Check the ISRTrace to determine which order the ISR Routines completed
	   asm("        ESTOP0");	   


// CASE 10:
//     Force all group 1 and group 2 and group 3 interrupts at once

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   
       
       // Enable PIE group 1, 2 and 3 interrupts 1-8 
       PieCtrlRegs.PIEIER1.all = 0x00FF;
       PieCtrlRegs.PIEIER2.all = 0x00FF;
       PieCtrlRegs.PIEIER3.all = 0x00FF;

	   // Make sure PIEACK for group 1, 2 & 3 are clear (default after reset)
       PieCtrlRegs.PIEACK.all = (M_INT3|M_INT2|M_INT3);

       // Enable CPU INT1, INT2 & INT3
       IER |= (M_INT1|M_INT2|M_INT3);
       
       // Force all valid interrupts for Group1, 2 and 3
       PieCtrlRegs.PIEIFR1.all = ISRS_GROUP1;  
       PieCtrlRegs.PIEIFR2.all = ISRS_GROUP2;  
       PieCtrlRegs.PIEIFR3.all = ISRS_GROUP3;  

	   // Enable Global interrupts
	   EINT;
	   
	   // Wait for all group 1 and group 2 and group 3 interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR1.all != 0x0000 
	      || PieCtrlRegs.PIEIFR2.all != 0x0000 
	      || PieCtrlRegs.PIEIFR3.all != 0x0000 ) {}
	      
	   // Check the ISRTrace to determine which order the ISR Routines completed
	   asm("        ESTOP0");	


// CASE 11:
//     Force all used PIE interrupts at once

       // Prepare for the test:
	   // Disable interrupts 
	   // Clear the trace buffer, PIE Control Register, CPU IER and IFR registers
	   DINT;
	   for(i = 0; i < 50; i++) ISRTrace[i] = 0;
	   ISRTraceIndex = 0;
       InitPieCtrl();
       IER = 0;
       IFR &= 0;

       // Enable the PIE block
       PieCtrlRegs.PIECRTL.bit.ENPIE = 1;   

       // Enable all PIE group iterrupts 1-8 
       PieCtrlRegs.PIEIER1.all = 0x00FF;
       PieCtrlRegs.PIEIER2.all = 0x00FF;
       PieCtrlRegs.PIEIER3.all = 0x00FF;
       PieCtrlRegs.PIEIER4.all = 0x00FF;
       PieCtrlRegs.PIEIER5.all = 0x00FF;
       PieCtrlRegs.PIEIER6.all = 0x00FF;
       PieCtrlRegs.PIEIER9.all = 0x00FF;

	   // Make sure PIEACK for group 1, 2, 3, 4, 5, 6 and 9 are clear (default after reset)
       PieCtrlRegs.PIEACK.all = (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT9);

       // Enable CPU INT1, INT2, INT3, INT4, INT5, INT6 and INT9
       IER |= (M_INT1|M_INT2|M_INT3|M_INT4|M_INT5|M_INT6|M_INT9);
              
       // Force all valid interrupts for all PIE groups
       PieCtrlRegs.PIEIFR1.all = ISRS_GROUP1;  
       PieCtrlRegs.PIEIFR2.all = ISRS_GROUP2;  
       PieCtrlRegs.PIEIFR3.all = ISRS_GROUP3;  
       PieCtrlRegs.PIEIFR4.all = ISRS_GROUP4;  
       PieCtrlRegs.PIEIFR5.all = ISRS_GROUP5;  
       PieCtrlRegs.PIEIFR6.all = ISRS_GROUP6;  
       PieCtrlRegs.PIEIFR9.all = ISRS_GROUP9;  

	   // Enable Global interrupts - CPU level
	   EINT;
	   
	   // Wait for all group interrupts to be serviced
	   while(PieCtrlRegs.PIEIFR1.all != 0x0000 
	      || PieCtrlRegs.PIEIFR2.all != 0x0000 
	      || PieCtrlRegs.PIEIFR3.all != 0x0000 
          || PieCtrlRegs.PIEIFR4.all != 0x0000 
	      || PieCtrlRegs.PIEIFR5.all != 0x0000 
	      || PieCtrlRegs.PIEIFR6.all != 0x0000 
	      || PieCtrlRegs.PIEIFR9.all != 0x0000 ) {}
	      
	   // Check the ISRTrace to determine which order the ISR Routines completed
	   asm("        ESTOP0");	

} 	

//===========================================================================
// No more.
//===========================================================================
