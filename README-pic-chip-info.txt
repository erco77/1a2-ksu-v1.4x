PIC CHIP PROGRAMMER: Ref: Hardware: https://www.youtube.com/watch?v=ksYe_FFAlEM
                          Software: https://www.youtube.com/watch?v=TIsiRmGVgUI


PICKIT 3 PROGRAMMER WIRING
                                                               _    _
            PIN1 >  o-----o MCLR              PIN2 -->     V+ | |__| | GND  <-- PIN3
               2    o-----o V+                             x  |      | DAT  <-- PIN4
               3    o-----o V-/GND                         x  |      | CLK  <-- PIN5
               4    o-----o PGM DATA          PIN1 -->  MCLR  |      | x
               5    o-----o PGM CLK                        x  |      | x
               6    o-----o AUX (don't connect)            x  |      | x
                                                           x  |      | x
                                                           x  |      | x
                                                           x  |      | x
                                                           x  |______| x

                                                             PIC16F1709

               PIC16F1709 COMPLETE PINOUT                    1A2 PIC CPU1 ASSIGNMENTS (08/07/2020)
               ==========================                    =====================================
                           _    _                                                _    _ 
                       V+ | |__| | GND                                       V+ | |__| | GND
                     RA5  |      | DAT / RA0           L1_RING_DET (IN) -- RA5  |      | RA0 -- (OUT) L1_LAMP (DAT)
                ____ RA4  |      | CLK / RA1           L1_LINE_DET (IN) -- RA4  |      | RA1 -- (OUT) L1_HOLD_RLY (CLK)
                MCLR/RA3  |      | RA2                    (MCLR) X (IN) -- RA3  |      | RA2 -- (OUT) RING_GEN_POW
                     RC5  |      | RC0                  L1_A_SENSE (IN) -- RC5  |      | RC0 -- (OUT) L2 HOLD_RLY
                     RC4  |      | RC1                  L2_A_SENSE (IN) -- RC4  |      | RC1 -- (OUT) BUZZ_RING
                     RC3  |      | RC2               SECONDARY_DET (IN) -- RC3  |      | RC2 -- (OUT) L1_RING_RLY
                     RC6  |      | RB4                 L2_LINE_DET (IN) -- RC6  |      | RB4 -- (OUT) L2_RING_RLY
                     RC7  |      | RB5             CPU_STATUS_LED (OUT) -- RC7  |      | RB5 -- (OUT) L2_LAMP
                     RB7  |______| RB6                 L2_RING_DET (IN) -- RB7  |______| RB6 -- (IN/OUT) SYNC_ILINK
                                                          
                         PIC16F1709                                         PIC16F1709 / CPU1
			                                                       REV G, H, J

        Software: 

	      MicroChip recommends "MPLAB X IDE Software",
		    which is what I ended up using.

	    Earlier I'd been interested in using MikroC PRO for PIC v.5.4.0,
	    as that had good online documentation + examples, but wasn't compatible
	    with MicroChip's newer tools for burning new chips.

        Xtal: 32.768kHz tuning fork type crystals for LP oscillator mode.
              Connect between SOSCO and SOSCI pins.

        Or, don't use any xtal (which is what I ended up doing), and just use the 
	internal oscillator by setting the config pragma at the top of the code:

                #pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)

        Reference Manual: http://ww1.microchip.com/downloads/en/DeviceDoc/40001729C.pdf

             p. 355 covers the PICKit programmer pinout (shown above)

             p. 2 gives overview of internal clock oscillator:
                 Clocking Structure
                 * 16 MHz Internal Oscillator Block:
                 * +/- 1% at calibration
                 * Selectable frequency range from 0 to 32 MHz
                 * 31 kHz Low-Power Internal Oscillator
                 * External Oscillator Block with:
                 * Three crystal/resonator modes up to 20 MHz
                 * Two external clock modes up to 20 MHz
                 * Fail-Safe Clock Monitor
                 * Two-Speed Oscillator Start-up
                 * Oscillator Start-up Timer (OST)

             p. 61-62 (Section 6.0 - 6.2) goes into more detail about the clock:

                 6.0 OSCILLATOR MODULE (WITH FAIL-SAFE CLOCK MONITOR)
                 ----------------------------------------------------

                     6.1 Overview
                     ------------
                     The oscillator module has a wide variety of clock sources and
                     selection features that allow it to be used in a wide range of
                     applications while maximizing performance and minimizing power
                     consumption. Figure 6-1 illustrates a block diagram of the oscillator
                     module.  Clock sources can be supplied from external oscillators,
                     quartz crystal resonators, ceramic resonators and Resistor-Capacitor
                     (RC) circuits. In addition, the system clock source can be supplied
                     from one of two internal oscillators and PLL circuits, with a choice
                     of speeds selectable via software. Additional clock features include:

                         * Selectable system clock source between external or internal sources via software.

                         * Two-Speed Start-up mode, which minimizes latency between external oscillator start-up and code execution.

                         * Fail-Safe Clock Monitor (FSCM) designed to detect a failure of the external clock source
                           (LP, XT, HS, ECH, ECM, ECL or EXTRC modes) and switch automatically to the internal oscillator.

                         * Oscillator Start-up Timer (OST) ensures stability of crystal oscillator sources.

                     The oscillator module can be configured in one of the following clock modes.

                         1. ECL - External Clock Low-Power mode (0 MHz to 0.5 MHz)
                         2. ECM - External Clock Medium-Power mode (0.5 MHz to 4 MHz)
                         3. ECH - External Clock High-Power mode (4 MHz to 32 MHz)
                         4. LP - 32 kHz Low-Power Crystal mode.
                         5. XT - Medium Gain Crystal or Ceramic Resonator Oscillator mode (up to 4 MHz)
                         6. HS - High Gain Crystal or Ceramic Resonator mode (4 MHz to 20 MHz)
                         7. EXTRC - External Resistor-Capacitor
                         8. INTOSC - Internal oscillator (31 kHz to 32 MHz)    *** INTERNAL OSCILLATOR RANGE ***

                     Clock Source modes are selected by the FOSC<2:0> bits in the Configuration Words.
                     The FOSC bits determine the type of oscillator that will be used when the device
                     is first powered.

                     The ECH, ECM, and ECL clock modes rely on an external logic level signal
                     as the device clock source.

                     The LP, XT, and HS clock modes require an external crystal or resonator
                     to be connected to the device.

                     Each mode is optimized for a different frequency range.
                     The EXTRC clock mode requires an external resistor and capacitor to set the oscillator frequency.
                     The INTOSC internal oscillator block produces low, medium, and high-frequency clock sources,
                     designated LFINTOSC, MFINTOSC and HFINTOSC.  (see Internal Oscillator Block, Figure 6-1).
                     A wide selection of device clock frequencies may be derived from these three clock sources.

                     ...

                     6.2 Clock Source Types
                     ----------------------
                     Clock sources can be classified as external or internal.

                     External clock sources rely on external circuitry for the
                     clock source to function. Examples are: oscillator modules
                     (ECH, ECM, ECL mode), quartz crystal resonators or ceramic resonators
                     (LP, XT and HS modes) and Resistor-Capacitor (EXTRC) mode circuits.

                     Internal clock sources are contained within the
                     oscillator module. The internal oscillator block has two
                     internal oscillators and a dedicated Phase Lock Loop
                     (HFPLL) that are used to generate three internal
                     system clock sources: the 16 MHz High-Frequency
                     Internal Oscillator (HFINTOSC), 500 kHz (MFINTOSC)
                     and the 31 kHz Low-Frequency Internal Oscillator (LFINTOSC).

                     The system clock can be selected between external or internal clock
                     sources via the System Clock Select (SCS) bits in the OSCCON register.
                     See Section 6.3 "Clock Switching" for additional information. 



             p. 49 covers clock config:

                 REGISTER 4-1: CONFIG1: CONFIGURATION WORD 1
                 bit 2-0 FOSC<2:0>: Oscillator Selection bits
                     111 = ECH: External Clock, High-Power mode (4-20 MHz): device clock supplied to CLKIN pin
                     110 = ECM: External Clock, Medium-Power mode (0.5-4 MHz): device clock supplied to CLKIN pin
                     101 = ECL: External Clock, Low-Power mode (0-0.5 MHz): device clock supplied to CLKIN pin
                     100 = INTOSC oscillator: I/O function on CLKIN pin
                     011 = EXTRC oscillator: External RC circuit connected to CLKIN pin
                     010 = HS oscillator: High-speed crystal/resonator connected between OSC1 and OSC2 pins
                     001 = XT oscillator: Crystal/resonator connected between OSC1 and OSC2 pins
                     000 = LP oscillator: Low-power crystal connected between OSC1 and OSC2 pins


       1A2 LINE CARD STATE FLOW CHART
       ==============================

       NOTE: For the latest version of this diagram, see "README-firmware-logic-diagram.txt"

       +-----------------------------------------+
       |                                         |
       |                                        / \
       |                                 ______/   \_____
       |                                /                \  yes
       |                               <   Line Detect?   >------------+
       |                                \______     _____/             |
       |                                       \   /                  / \
       |                                        \ / no         ______/   \_____
       |                                         |            /                \  yes
       |                                         |           <       Hold?      >--------------------------------+  (Currently on HOLD)
       |                                        / \           \______     _____/                                 |
       |                                 ______/   \_____            \   /                                      / \
       |                            yes /                \            \ / no                             ______/   \_____
       |                   +-----------<       Ring?      >            |  (On Call)                     /                \  yes
       |    (Line Ringing) |            \______     _____/             |                               <      A Lead?     >-----------+
       |                   |                   \   /                  / \                               \______     _____/            |
       |                   |                    \ / no         ______/   \_____                                \   /                  | (Pickup held call)
       |                   |                     |  (Idle)    /                \ yes                            \ / no                |
       |                   |                     |           <      A Lead?     >-----------+                    |  (On HOLD)         |
       |                   |                     |            \______     _____/            | (On Call)          |                    |
       |                   |                     |                   \   /                  |                    |                    |
       |                   |                     |                    \ / no                |                    |                    |
       |                   |                     |                     |  (Pushed HOLD)     |                    |                    |
       |                  \|/                   \|/                   \|/                  \|/                  \|/                  \|/
       |              Line Ringing           Line Idle        Transition To HOLD       Line In Use         Call On HOLD       Pickup From HOLD
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |           |  0 -> Hold Rly  |   |  0 -> Hold Rly  |  |  1 -> Hold Rly  |  |  0 -> Hold Rly  |  |  1 -> Hold Rly  |  |  0 -> Hold Rly  |
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |                   |                     |                     |                    |                    |                    |
       |                  \|/                   \|/                   \|/                  \|/                  \|/                  \|/
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |           |  0 -> Hold State|   |  0 -> Hold State|  |  1 -> Hold State|  |  0 -> Hold State|  |  0 -> Hold State|  |  0 -> Hold State|
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |                   |                     |                     |                    |                    |                    |
       |                  \|/                   \|/                   \|/                  \|/                  \|/                  \|/
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |           | RCAD -> Ring Rly|   |  0 -> Ring Rly  |  |  0 -> Ring Rly  |  |  0 -> Ring Rly  |  |  0 -> Ring Rly  |  |  0 -> Ring Rly  |
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |                   |                     |                     |                    |                    |                    |
       |                  \|/                   \|/                   \|/                  \|/                  \|/                  \|/
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |           | Ring -> Lamp    |   |  0 -> Lamp      |  | Hold -> Lamp    |  |  1 -> Lamp Xstr |  |  Hold -> Lamp   |  |  1 -> Lamp Xstr |
       |           +-----------------+   +-----------------+  +-----------------+  +-----------------+  +-----------------+  +-----------------+  
       |                   |                     |                     |                    |                    |                    |
       |                  \|/                   \|/                   \|/                  \|/                  \|/                  \|/
       |                   |                     |                     |                    |                    |                    |
       +-------------------o---------------------o---------------------o--------------------o--------------------o--------------------o

    NOTE: For the latest version of the following code, see the actual source code
    used for programming CPU#1 in "1a2-pic-cpu1-REV-G-firmware.c.txt". The following
    was the initial code implementation, put here for record keeping of the initial plan,
    (which evolved over several revisions):

typedef struct {
    // Inputs (debounced)
    char line_det  = 0;
    char ring_ret  = 0;
    char a_lead    = 0;
    // Outputs
    char hold_rly  = 0;
    char ring_rly  = 0;
    char lamp_xstr = 0;
    // State
    char hold_state = 0;
} Line;

// Lamp cadences
char hold_lamp[] = { 0,1,1,1,1,1,0,1,1,1,1,1 }; // 1 sec overall (~83 msec each)
char ring_lamp[] = { 0,0,0,0,0,0,1,1,1,1,1,1 }; // 1 sec overall (~83 msec each)
char ring_bell[] = { 1,1,0,0,0,0 };             // 6 sec overall (1 sec each)

// Handle input transitions. Reads the 3 inputs: line detect, ring detect, and A lead.
// Given that, manage the outputs: lamp, hold relay, ring relay.
//
void HandleLineTransitions(line) {
   if ( line->line_det ) {
       // Call in progress
       if ( line->hold_state ) {
           // Call On HOLD
           if ( line->a_lead ) {
               // Started picking up a HOLD call
               line->hold_rly   = 0;
               line->hold_state = 0;
               line->ring_rly   = 0;
               line->lamp_xstr  = hold_lamp[counter % sizeof(hold_lamp)];
           } else {
               // Still on HOLD
               line->hold_rly   = 1;
               line->hold_state = 0;
               line->ring_rly   = 0;
               line->lamp_xstr  = hold_lamp[counter % sizeof(hold_lamp)];
           }
       } else {
           // On call (not HOLD)
           if ( line->a_lead ) {
               // On call/line in use
               line->hold_rly   = 0;
               line->hold_state = 0;
               line->ring_rly   = 0;
               line->lamp_xstr  = 1;
           } else {
               // Transition to HOLD (A lead just released)
               line->hold_rly   = 1;
               line->hold_state = 1;
               line->ring_rly   = 0;
               line->lamp_xstr  = hold_lamp[counter % sizeof(hold_lamp)];
           }
       }
   } else {
       // Here if: idle, ringing, dropped from hold, someone hung up, CPC
       if ( line->ring ) {
           // Line ringing
           hold_rly   = 0;
           hold_state = 0;
           ring_rly   = ring_bell[counter % sizeof(ring_bell));
           lamp_xstr  = ring_lamp[counter % sizeof(ring_lamp)];
       } else {
           // Line idle
           hold_rly   = 0;
           hold_state = 0;
           ring_rly   = 0;
           lamp_xstr  = 0;
       }
   }
}


WHERE TO GET THE SOFTWARE
-------------------------


   MPLAB
   =====
   This is what I ended up using along with the MPLAB X IDE on Windows:
   C compiler:
   https://www.microchip.com/mplab/compilers

   MikroC
   ======
   In the end, I didn't use this toolkit, but it's here for reference.
   https://www.mikroe.com/blog/compiler-quick-start-guide

   Perhaps a way to get MikroC to write .HEX file to PICKit3:
   http://www.theengineeringprojects.com/2013/03/how-to-burn-mikroc-code-using-pickit3.html
   ..but in the end I couldn't get that to work, so I started using the MPLAB X IDE,
   which is kinda slow and uses up a lot of memory on my two Windows 7 and Windows 8
   machines. Upgrading to a Windows 7 machine with more memory helped a lot for responsiveness.
   Windows 8 is a shit version of the OS, I don't recommend it. The Metro interface gets in the
   way of everything.


---------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------

    After much histrionics that made the cat leave the room,
    I got two LEDs blinking properly with my PIC16F1709's using
    Microchip's MPLABX IDE environment.

    Bought the PIC chips and a "PICKit 3" programmer from Digikey,
    and the rest was free software downloads of Microchip's IDE.

    The led flashing code ended up being:
    _______________________________________________________________________________________________________________

    /*
     * File:   main.c
     * Author: erco
     *
     * Created on January 6, 2019, 2:37 PM
     */

    #define _XTAL_FREQ 1000000UL        // 4MHz/4?

    // -START BLOCK- //
    // The following code block is a copy/paste from MPLABX's menu:
    // Production -> Set Configuration Bits -> Generate Source Code to Output

    // CONFIG1
    #pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
    #pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
    #pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
    #pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
    #pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
    #pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
    #pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
    #pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
    #pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

    // CONFIG2
    #pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
    #pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit cannot be cleared once it is set by software)
    #pragma config ZCDDIS = ON      // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR)
    #pragma config PLLEN = OFF      // Phase Lock Loop enable (4x PLL is enabled when software sets the SPLLEN bit)
    #pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
    #pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
    #pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
    #pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

    // -END BLOCK- //

    // #pragma config statements should precede project file includes.
    // Use project enums instead of #define for ON and OFF.

    // Ref: https://electronics.stackexchange.com/questions/171530/pic-microchip-keeps-resetting
    #include <xc.h>
    // #include <pic16f1709.h>  // the chip I'm using. xc.h gets the chip info from the IDE's project settings

    void main(void) {
        // Set PIC chip oscillator speed
        OSCCONbits.IRCF   = 0b1011;  // 0000=31kHz LF, 0111=500kHz MF (default on reset), 1011=1MHz HF, 1101=4MHz, 1110=8MHz, 1111=16MHz HF
        OSCCONbits.SPLLEN = 0;       // disable 4xPLL (PLLEN in config words must be OFF)
        OSCCONbits.SCS    = 0b10;    // 10=int osc, 00=FOSC determines oscillator
        // Set all data direction bits to outputs (0=out, 1=in)
        // Example, TRISA bit 0 is RA0 port, TRISA bit 1 is RA1 port, etc.
        TRISA = 0x0;
        TRISB = 0x0;
        TRISC = 0x0;
        while (1) {
            PORTB = 0xaa;  __delay_ms(500);     // sets all even bits in port B, delay 1/2 sec
            PORTB = 0x55;  __delay_ms(500);     // sets all odd  bits in port B, delay 1/2 sec
        }
    }
    _______________________________________________________________________________________________________________

    ..and the LED wiring ended up being:

                                       _    _
                      5VDC <------ V+ | |__| | GND --------> GND
                                 RA5  |      | DAT / RA0
                            ____ RA4  |      | CLK / RA1
        5VDC <--(47k RES)-- MCLR/RA3  |      | RA2
                                 RC5  |      | RC0
                                 RC4  |      | RC1
                                 RC3  |      | RC2
                                 RC6  |      | RB4
                                 RC7  |      | RB5 --------(LED)-----(220 OHM RES)-----> GND
                                 RB7  |______| RB6 --------(LED)-----(220 OHM RES)-----> GND

                                     PIC16F1709

    ..where the LEDs flash in an alternating pattern.
    And the programmer wiring (overlayed with the above) being:
     
                PICKit 3 PROGRAMMER PINOUT
                ==========================
                                                                     _    _
                PIN1 >  o-----o MCLR                PIN2 -->     V+ | |__| | GND  <-- PIN3
                   2    o-----o V+                               x  |      | DAT  <-- PIN4
                   3    o-----o V-/GND                           x  |      | CLK  <-- PIN5
                   4    o-----o PGM DATA            PIN1 -->  MCLR  |      | x
                   5    o-----o PGM CLK                          x  |      | x
                   6    o-----o AUX (don't connect)              x  |      | x
                                                                 x  |      | x
                                                                 x  |      | x
                                                                 x  |      | x
                                                                 x  |______| x

                                                                   PIC16F1709

    <digression>

    Most of the hassle for me was the Mac version of MPLABX was constantly
    crashing.

    I finally moved to the Windows version which didn't crash, but used
    so much ram it ran really slow on my 2 Gig Windows 8 machine.

    Also, it didn't help that the 16F1709 is a relatively new chip,
    so there weren't any simple "hello world" examples for it with this
    compiler.

    So I was floundering with that for a while, getting weird behavior
    before I found the proper configuration bits needed to get consistent
    behavior, as the chip was auto-resetting (fixed by WDTE=OFF), and timing
    was bad due to the PLL (phase locked loop) modules being enabled by default
    (fixed by PLLEN=OFF).

    While I ended up using Microchip's bloated Java based MPLABX IDE
    and their XC8 compiler (separate download), I'd actually wanted
    to use the MikroC PIC compiler instead, which has great online help
    and a fast/small IDE. But they didn't have an easy way to invoke
    the PICKit 3 programmer to upload the code to the chip.

    Apparently the approach was to configure MikroC to invoke Microchip's
    "standalone programmer", or "PIC Kit Programmer", which was simple
    and small and easy, but Microchip says they stopped supporting it
    in favor of their massive "MPLAB IPE" instead, another bloated
    java application similar to the "MPLAB IDE".

    So all the standalone PIC programmer apps I could find from Microchip
    were frozen in time, old enough not to include my relatively new chip
    in their pulldown menu list of devices :/

    So I ended up sticking with Microchip's MPLAB IDE + Compiler,
    and rewrote my MikroC code to use their programming model instead.
    (Different macros, bad docs.. bleh)
    </digression>

    PIC CHIP INPUT/OUTPUTS
    ----------------------
    Notes about PIC chip inputs/outputs..

    PIC INPUTS
    ==========

        To /read/ a particular port bit, e.g. port A bit #0, one can use:

            val = PORTAbits.RA0;                // read bit #0

               - or -

            val = (PORTA & 0x00000001); // read all 8 bits, isolate bit#0

        To avoid time aliasing, it's probably best in your main() loop to read
        all the input port values once into some global variables, and then test
        the bits of those during the loop iteration, e.g.

        int main() {
            while ( 1 ) {
                G_porta = PORTA;
                G_portb = PORTB;
                G_portc = PORTC;

                // Do stuff reading G_porta/b/c only, not live hardware

                __delay_ms(..);
            } // while
        } // main

        Weak Pullup Resistors
        ---------------------
        Inputs can have internal "weak" pullup resistors assigned to each. To do this,
        when setting up the port direction bits, you can also turn on weak pullups
        for which ever inputs you want, e.g. in the following PORTC bits 0-3 are programmed
        to be inputs, but only bits 0 and 2 are configured with 'weak pullups' enabled:

            OPTION_REGbits.nWPUEN = 0;  // enable WPUEN (weak pullup enable) by /clearing/ the bit
            ..
            TRISC = 0b00001111;         // program PORTC bits 0-3 as inputs
            WPUC  = 0b00000101;         // enable weak pullups for bits 0 and 2

        Schmitt Inputs
        --------------
        Inputs can be assigned to either be Schmitt Triggered, or the default TTL style inputs.
        Here, Schmitt is disabled for all ports:

            // Normal input thresholds
            //     1: Schmitt TRigger input used for PORT reads and interrupt-on-change
            //     0: TTL input used for PORT reads and interrupt-on-change
            //
            INLVLA = 0b00000000;
            INLVLB = 0b00000000;
            INLVLC = 0b00000000;

        Analog Inputs
        -------------
        Inputs can be assigned to read values in analog. See docs for more, but if you're
        not using analog inputs, it's probably best to explicitly disable analog stuff:

            ANSELA = 0b00000000;
            ANSELB = 0b00000000;
            ANSELC = 0b00000000;
            ADCON0 = 0x0;       // Disable ADC

        Comparator
        ----------
        PIC also supports comparator inputs. See docs for more, but if you're not
        going to be using the comparator feature, it's probably best to disable it explicitly:

            CM1CON0 = 0x0;
            CM2CON0 = 0x0;
            CM1CON1 = 0x0;
            CM2CON1 = 0x0;

    PIC OUTPUTS
    ===========

        To /write/ to a particular port bit, e.g. port A bit #0, one can use:

            LATAbits.LATA0 = 1;         // set single bit in PORTA

                - or -

            PORTA = 0000000001;         // set all 8 bits in PORTA
        
        Outputs can either be 'open drain' (the MOS equivalent of "open collector"?) or normal
        push-pull drive (default I think). Here I'm setting all output bits to be push-pull.
        Some bits will be 'don't care' because bit settings are only valid for port bits
        programmed to be outputs. Note too not all PORT bits are available for I/O, only the
        ones your chip supports:

            // Disable 'Open Drain' control:
            //     1: open drain drive (sink current only)
            //     0: normal push-pull drive (source and sink current)
            //
            ODCONA = 0b00000000;        
            ODCONA = 0b00000000;        
            ODCONA = 0b00000000;

---------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------
1A2 PIC 
    
    --- debounce:
    Set up a 100hz interrupt and sample the pin every time. Every time
    you see it high inc a counter, When its low zero it. Once you see
    the counter over x then set another flag to say the input was pressed.

    --- "weak pullup":
    PIC internal "weak pull up" resistors are ~20k.  From the data sheet:
        """
        D070 IPURB PORTB Weak Pull-up Current 50 250 400 uA VDD = 5V,
                         VPIN = VSS, -40 deg C TO +85 deg C
        """
    ..The typical value of 250 microamps is equivalent to a pull up
    resistance of 5/.00025 Ohms = 20K......maybe a bit on the higher side
    for an EMI prone application.

    Inputs:
        Line Detect
        Ring Detect
        A Lead

    Outputs:
        Lamp
        Hold relay
        Ring relay

    Main Loop
    ---------

        o Hold flash/Ring flash on/off bit
          --------------------------------

          Manage this as a binary integer counter that runs at 1/8 second rate:

                               1 sec
                        |----------------|
                   Bit0: __--__--__--__--__--__--__--__--
                        :                :
                   Bit1: ____----____----____----____----
                        :                :
                   Bit2: ________--------________--------
                        :                :
                   Hold: __------__------__------__------  (Bit0|Bit1)
                        :                :
                   Ring: ________--------________--------  (Bit3)


        o Line Detect
          -----------
          Manage with a state flag that's either IDLE (0) or BUSY(1).
          Debounce before making a change in state by checking for several samples.

          The state will be used for:

              o Lamp status (off/on/flashing during ringing/flashing during HOLD)
              o Hold detection (when A lead drops but line detect still shows BUSY for >.1sec)
              o Stops ringing (audible and ring flash) when someone picks up
              o Releases "Hold" on CPC

          Should be handled as an interrupt, so the chip can sleep most of the time,
          and only be "awake" when a line is "BUSY" (with ringing or line detect/hold)

        o Ring Detect
          -----------
          Handle this by debouncing.

          
          On ring, assuming line detect is IDLE, 
          Should be handled as an interrupt, so the chip can sleep
          most of the time, and only be "awake" when a line is active.

        o Sleep mode: only when:

              "Line Detect" state is IDLE
              "Ringing" state is IDLE.

---
    PIC Related Videos (list of different videos):
    https://microchip.secure.force.com/microchipknowledge/articles/en_US/FAQ/Learning-PIC-Microcontrollers-Videos/?l=en_US&fs=Search&pn=1

    ..Some specific ones to watch:

        Begin Programming A PIC16F1xxx in C like a Pro
        https://www.youtube.com/watch?v=Lm6e3xSC8sg

        Taming Embedded C - Pt 1
        https://www.youtube.com/watch?v=BtXyvVy67Qs

    Q/A: Where can I find microcontroller code examples (PIC16F, etc)
    https://microchip.secure.force.com/microchipknowledge/articles/en_US/FAQ/Where-to-find-code-examples-and-libraries-for-Microchip-microcontrollers-PIC16F-PIC18F-PIC24-dsPIC-etc/?l=en_US&fs=Search&pn=1

    Some specific pages:

        1. Go to http://www.microchip.com/mplab/microchip-libraries-for-applications 
        2. Download the latest version of MLA (Microchip Libraries for Applications). 
        3. The C:\microchip\mla\v2016_11_07\doc folder (or whichever current version of MLA),
           will have various help documents. 

    16F1709 video:
    https://player.vimeo.com/video/204700953

        Some applications are available only in the Legacy MLA (v2013-06-15),
        so it is recommended to install and check also there.

        In order to download Legacy MLA, go to:
        http://www.microchip.com/mplab/microchip-libraries-for-applications
        ..then scroll-down, select "Legacy MLA" tab and choose the package
        specific to your operating system.

        Note: The MLA is moving to MPLAB Code Configurator (MCC) libraries.

        The old Microchip Peripheral Libraries can be downloaded
        from the website, however, the page mentioned above is now
        obsolete. Therefore, no further updates will be made to those
        libraries. We recommend to move on to MCC and use it for all
        your future projects.

        To set up the registers of a Microchip device,
        you can download MPLAB Code Configurator (MCC) from
        http://www.microchip.com/mplab/mplab-code-configurator
        or by MPLAB X IDE -> Tools ->Plugins. This tool can easily set
        up the registers and peripherals, once it is installed. Check
        if your device is supported by MCC before you try to configure it.

        For additional general MCC information visit
        http://microchipdeveloper.com/mcc:overview

        How to use the IPE video:
        https://vimeo.com/307355242?from=outro-embed

      
      Timers
      ======
      Timer in our case is driven by the instruction clock,
      which is 1/4 the xtal oscillator frequency (_XTAL_FREQ),
      which we'll call FOSC4.

      So, the formula for calculating the number of interrupts
      per second for different prescalar values:

        #define _XTAL_FREQ 4000000UL    // 4MHz
        #define FOSC4 (_XTAL_FREQ/4)    // 1MHz

        ints_per_sec = FOSC4 / prescalar / 256;                 // 256: is the 8 bit counter that triggers the
                                                                //      interrupt each time it overflows

      ..where 'prescalar' is based on this table for PS<2:0> values:
        
        PS<2:0> |  Prescalar value
        --------|-----------------
          000   |   /2
          001   |   /4
          010   |   /8
           :         :
          111   |  /256

      So if PS<2:0> is set to 111, the formula ends up being:

         ints_per_sec = FOSC4 / 256 / 256
         ints_per_sec = 1000000 / 256 / 256
         ints_per_sec = 15.26

      The simplified logic diagram for TMR0 if configured to run
      of the internal instruction clock:
         
         Set INTCON     TMR0IE (IE=int enable) to 1 to enable timer0 interrupt.
         Set OPTION_REG TMR0CS (CS=Clock Select?) to 0 to enable FOSC4 to drive ints.
         Set OPTION_REG PSA (PSA=Prescalar) to 1 to enable prescalar.
         Set TMR0SE (SE=Select Edge?) selects rising or falling edge to trigger ints (who cares)

          _______         __________         _______________________________
         |       \       |          \       | 8 bit timer/counter register  \
         | FOSC/4 >------| Prescalar >------| (basically divides by 256)     >------------> Int Trigger
         |_______/       |__________/       |_______________________________/  overflow
                                                 counts 0-ff, then overflows


      See datasheet for INTCON register: (search for intcon)

          GIE | PEIE | TMR0IE | INTE | RBIE | TMR0IF | INTF | RBIF
                         |
                         1 enables the TMR0 int,
                         0 disables the TMR0 interrupt

      Whenever TMR0's 0-ff counter overflows, the TMR0IF (IF=int flag)
      is set, and an interrupt is triggered. Software can inspect the flag
      to see if this interrupt was generated by TMR0 (if not, the flag would
      be 0, indicating the cause was something else)

      Going in or out of sleep mode disables the timer.
      If you want the timer running during sleep mode, you have to re-enable
      timer ints on to-sleep or from-sleep; see data sheet.

      So, the following code is from the video on interrupts:

      // Interrupt handler (ISR == interrupt service request?)
      //
      // Video's calculation:
      //     Timer at 1:256 clock frequency means:
      //     freq_out = freq_clk / (4*prescalar*(256-TMR0)*K)
      //     freq_out = 20MHz / (4*256*(256-0)*K
      //     K = 20,000,000 / (4*256*256) = 76 (approx) ints per sec
      //
      //     For 4MHz xtal, the calculation is:
      //     K = 4,000,000 / (4*256*256) = 15.26
      //             \         \  \   \__ 8 bit 0-ff counter
      //              \         \  \__ prescalar
      //               \         \__ instruction clock is XTAL_FREQ/4
      //                \__ XTAL_FREQ
      //
      int icount = 0;           // our interrupt counter (0-15 == ~1 sec)
      void interrupt ISR(void) {
          if ( INTCONbits.TMR0IF ) {    // see if timer overflow occurred
              INTCONbits.TMROIF = 0;    // if so, clear timer overflow flag
              icount++;
              if ( icount == 76 ) {
                  icount = 0;
                  // DO INTERRUPT STUFF HERE FOR ONCE A SECOND
                  PORTA = ~PORTA;       // flip all bits of portD
              }
          }
      }

      INTCONbits.GIE = 1;    // enable all global interrupts
      INTCONbits.PEIE = 1;   // enable peripheral ints
      INTCONbits.TMR0IE = 1; // enable timer int
      INTCONbits.TMR0IF = 0; // cleartimer overflow flag
      ei();

      // Configure timer
      //    Search datasheet for for option_reg to set the
      //    1) clock source (TOCS:TMR0 clock source, and 2) prescalar (PS2:PS0)
      //
      OPTION_REGbits.T0CS = 0;  // internal instruction cycle clock (CLK0)
      OPTION_REGbits.T0SE = 0;  // edge selection
      OPTION_REGbits.PSA  = 0;  // assign prescalar to timer 0
      OPTION_REGbits.PS0  = 1;  // --
      OPTION_REGbits.PS1  = 1;  //  |-- together selects 1:256
      OPTION_REGbits.PS2  = 1;  // --

*******************************************************************************************

    The following are excerpts from Microchip's compiler docs, included here
    for quick reference: http://ww1.microchip.com/downloads/en/devicedoc/50002053g.pdf
    This is reformatted slightly to be readable as pure text.

*******************************************************************************************


4.3.5 Configuration Bit Access
------------------------------
Configuration bits or fuses are used to set up fundamental device operation, such as
the oscillator mode, watchdog timer, programming mode and code protection. These
bits must be correctly set to ensure your program executes correctly.

Use the configuration pragma, which has the following forms, to set up your device.

    #pragma config setting = state|value
    #pragma config register = value

[..]

The settings and states associated with each device can be determined from
an HTML guide. Open the pic_chipinfo.html file (or the pic18_chipinfo.html
file) that is located in the pic/docs directory of your compiler
installation.  

    > ERCO: This is actually in the xc8\v#.##\docs\chips\*.html directory.
    > ERCO: In my case, with the 16f1709, the docs for that are:

. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

16F1709 Support Information

Register: CONFIG1 @ 0x8007
    FOSC - Oscillator Selection Bits
        ECH     ECH, External Clock, High Power Mode (4-20 MHz): device clock supplied to CLKIN pins
        ECM     ECM, External Clock, Medium Power Mode (0.5-4 MHz): device clock supplied to CLKIN pins
        ECL     ECL, External Clock, Low Power Mode (0-0.5 MHz): device clock supplied to CLKIN pins
        INTOSC  INTOSC oscillator: I/O function on CLKIN pin
        EXTRC   EXTRC oscillator: External RC circuit connected to CLKIN pin
        HS      HS Oscillator, High-speed crystal/resonator connected between OSC1 and OSC2 pins
        XT      XT Oscillator, Crystal/resonator connected between OSC1 and OSC2 pins
        LP      LP Oscillator, Low-power crystal connected between OSC1 and OSC2 pins

    WDTE - Watchdog Timer Enable
        ON      WDT enabled
        NSLEEP  WDT enabled while running and disabled in Sleep
        SWDTEN  WDT controlled by the SWDTEN bit in the WDTCON register
        OFF     WDT disabled

    PWRTE - Power-up Timer Enable
        OFF     PWRT disabled
        ON      PWRT enabled

    MCLRE - MCLR Pin Function Select
        ON      MCLR/VPP pin function is MCLR
        OFF     MCLR/VPP pin function is digital input

    CP - Flash Program Memory Code Protection
        OFF     Program memory code protection is disabled
        ON      Program memory code protection is enabled

    BOREN - Brown-out Reset Enable
        ON      Brown-out Reset enabled
        NSLEEP  Brown-out Reset enabled while running and disabled in Sleep
        SBODEN  Brown-out Reset controlled by the SBOREN bit in the BORCON register
        OFF     Brown-out Reset disabled

    CLKOUTEN - Clock Out Enable
        OFF     CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
        ON      CLKOUT function is enabled on the CLKOUT pin

    IESO - Internal/External Switchover Mode
        ON      Internal/External Switchover Mode is enabled
        OFF     Internal/External Switchover Mode is disabled

    FCMEN - Fail-Safe Clock Monitor Enable
        ON      Fail-Safe Clock Monitor is enabled
        OFF     Fail-Safe Clock Monitor is disabled

Register: CONFIG2 @ 0x8008
    WRT - Flash Memory Self-Write Protection
        OFF     Write protection off
        BOOT    000h to 1FFh write protected, 200h to 1FFFh may be modified by EECON control
        HALF    000h to FFFh write protected, 1000h to 1FFFh may be modified by EECON control
        ALL     000h to 1FFFh write protected, no addresses may be modified by EECON control

    PPS1WAY - Peripheral Pin Select one-way control
        ON      The PPSLOCK bit cannot be cleared once it is set by software
        OFF     The PPSLOCK bit can be set and cleared repeatedly by software

    ZCDDIS - Zero-cross detect disable
        ON      Zero-cross detect circuit is disabled at POR
        OFF     Zero-cross detect circuit is enabled at POR

    PLLEN - Phase Lock Loop enable
        ON      4x PLL is always enabled
        OFF     4x PLL is enabled when software sets the SPLLEN bit

    STVREN - Stack Overflow/Underflow Reset Enable
        ON      Stack Overflow or Underflow will cause a Reset
        OFF     Stack Overflow or Underflow will not cause a Reset

    BORV - Brown-out Reset Voltage Selection
        LO      Brown-out Reset Voltage (Vbor), low trip point selected.
        HI      Brown-out Reset Voltage (Vbor), high trip point selected.

    LPBOR - Low-Power Brown Out Reset
        OFF     Low-Power BOR is disabled
        ON      Low-Power BOR is enabled

    LVP - Low-Voltage Programming Enable
        ON      Low-voltage programming enabled
        OFF     High-voltage on MCLR/VPP must be used for programming
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

    > ERCO: Continuing the compiler docs..

    Click the link to your target device and the page will show you the settings and values
    that are appropriate with this pragma. Review your device data sheet for more information.

    The value field is a numerical value that can be used in preference to a descriptor.
    Numerical values can only be specified in decimal or in hexadecimal, the latter radix
    indicated by the usual 0x prefix. Values must never be specified in binary (i.e., using
    the 0b prefix).

    Consider the following examples.

        #pragma config WDT = ON // turn on watchdog timer
        #pragma config WDTPS = 0x1A // specify the timer postscale value

    One pragma can be used to program several settings by separating each setting-value
    pair with a comma. For example, the above could be specified with one pragma, as in
    the following.

        #pragma config WDT=ON, WDTPS = 0x1A

    It is recommended that the setting-value pairs be quoted to ensure that the preprocessor
    does not perform substitution of these tokens, for example:

        #pragma config "BOREN=OFF"

    You should never assume that the OFF and ON tokens used in configuration macros
    equate to 0 and 1, respectively, as that is often not the case.

    Rather than specify individual settings, each half of the configuration register can be
    programmed with one numerical value, for example:

        #pragma config CONFIG1L = 0x8F

    Neither the config pragma nor the __CONFIG macro produce executable code, and
    ideally they should both be placed outside function definitions.

    All the bits in the Configuration Words should be programmed to prevent erratic program
    behavior. Do not leave them in their default/unprogrammed state. Not all Configuration
    bits have a default state of logic high; some have a logic low default state.
    Consult your device data sheet for more information.

    If you are using MPLAB X IDE, take advantage of its built-in tools to generate the
    required pragmas, so that you can copy and paste them into your source code.


    4.3.6 ID Locations
    ------------------

    The 8-bit PIC devices have locations outside the addressable memory area that can be
    used for storing program information, such as an ID number. The config pragma is
    also used to place data into these locations by using a special register name. The
    pragma is used as follows:

        #pragma config IDLOCx = value

    where x is the number (position) of the ID location, and value is the nibble or byte that
    is to be positioned into that ID location. The value can only be specified in decimal or
    in hexadecimal, the latter radix indicated by the usual 0x prefix. Values must never be
    specified in binary (i.e., using the 0b prefix). If value is larger than the maximum value
    allowable for each location on the target device, the value will be truncated and a warning
    message is issued. The size of each ID location varies from device to device. See
    your device data sheet for more information. For example:

        #pragma config IDLOC0 = 1
        #pragma config IDLOC1 = 4

    will attempt fill the first two ID locations with 1 and 4. One pragma can be used to program
    several locations by separating each register-value pair with a comma. For
    example, the above could also be specified as shown below.

        #pragma config IDLOC0 = 1, IDLOC1 = 4

    The config pragma does not produce executable code and so should ideally be placed
    outside function definitions.


    4.3.7 Using SFRs From C Code
    ----------------------------

    The Special Function Registers (SFRs) are typically memory mapped registers and are
    accessed by absolute C structure variables that are placed at the register’s address.
    These structures can be accessed in the usual way so that no special syntax is required
    to access SFRs.

    The SFRs control aspects of the MCU and peripheral module operation. Some registers
    are read-only; some are write-only. Always check your device data sheet for complete
    information regarding the registers.

    The SFR structures are predefined in header files and are accessible once you have
    included <xc.h> (see Section 4.3.3 “Device Header Files”) into your source files.
    Structures are mapped over the entire register and bit-fields within those structures
    allow access to specific SFR bits. The names of the structures will typically be the same
    as the corresponding register, as specified in the device data sheet, followed by bits
    (see Section 2.4.2.5 “How Do I Find The Names Used to Represent SFRs and Bits?”).
    For example, the following shows code that includes the generic header file, clears
    PORTA as a whole and sets bit 2 of PORTA using the bit-field definition.

        #include <xc.h>
        void main(void)
        {
            PORTA = 0x00;
            PORTAbits.RA2 = 1;
        }

    Care should be taken when accessing some SFRs from C code or from in-line assembly.
    Some registers are used by the compiler to hold intermediate values of calculations,
    and writing to these registers directly can result in code failure. A list of registers
    used by the compiler and can be found in Section 4.7 “Register Usage”.

    4.3.7.1 SPECIAL PIC18 REGISTER ISSUES
    -------------------------------------

    Some of the SFRs used by PIC18 devices can be grouped to form multi-byte values,
    e.g., the TMRxH and TMRxL register combine to form a 16-bit timer count value.
    Depending on the device and mode of operation, there can be hardware requirements
    to read these registers in certain ways, e.g., often the TMRxL register must be read
    before trying to read the TMRxH register to obtain a valid 16-bit result.

    It is not recommended that you read a multi-byte variable mapped over these registers
    as there is no guarantee of the order in which the bytes will be read. It is recommended
    that each byte of the SFR should be accessed directly, and in the required order, as
    dictated by the device data sheet. This results in a much higher degree of portability.
    The following code copies the two timer registers into a C unsigned variable count
    for subsequent use.

        count = TMR0L;
        count += TMR0H << 8;

    Macros are also provided to perform reading and writing of the more common timer registers.
    See the macros READTIMERx and WRITETIMERx in Appendix A. Library Functions.
    These guarantee the correct byte order is used.

    4.3.8 Bit Instructions
    ----------------------

    Wherever possible, the MPLAB XC8 C Compiler will attempt to use bit instructions,
    even on non-bit integer values. For example, when using a bitwise operator and a mask
    to alter a bit within an integral type, the compiler will check the mask value to determine
    if a bit instruction can achieve the same functionality.

        unsigned int foo;
        foo |= 0x40;

    ..will produce the instruction:

        bsf _foo,6

    To set or clear individual bits within integral type, the following macros could be used:

        #define bitset(var, bitno) ((var) |= 1UL << (bitno))
        #define bitclr(var, bitno) ((var) &= ~(1UL << (bitno)))

    To perform the same operation on foo as above, the bitset macro could be
    employed as follows:

        bitset(foo, 6);

    4.3.10 Baseline PIC MCU Special Instructions
    --------------------------------------------

    Baseline devices can use the OPTION and TRIS SFRs, which are not memory
    mapped.

    The definition of these registers use a special qualifier, __control, to indicate that the
    registers are write-only, outside the normal address space, and must be accessed
    using special instructions.

    When these SFRs are written in C code, the compiler will use the appropriate instruction
    to store the value. So, for example, to set the TRIS register, the following code:

        TRIS = 0xFF;

    would be encoded by the compiler as:

        movlw 0ffh
        TRIS

    Those Baseline PIC devices which have more than one output port can have
    __control definitions for objects: TRISA, TRISB and TRISC, and which are used in
    the same manner as described above.

    Any register that uses the __control qualifier must be accessed as a full byte. If you
    need to access bits within the register, copy the register to a temporary variable first,
    then access that temporary variable as required.

    ------------------------------------------------------------------------------------------------------
                XC8 COMPILER PIC RELATED LIBRARY FUNCTIONS: #include <xc.h>
    ------------------------------------------------------------------------------------------------------

                            TABLE A-18: DECLARATIONS PROVIDED BY <XC.H>
    :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    :: Name                            Definition                                                        ::
    :: ------------------              ----------------------------------------                          ::
    :: di                              -                                                                 ::
    :: ei                              -                                                                 ::
    :: CRLWDT                          -                                                                 ::
    :: EEPROM_READ                     EEPROM_READ(address)                                              ::
    :: EEPROM_WRITE                    EEPROM_WRITE(address, value)                                      ::
    :: NOP                             -                                                                 ::
    :: READTIMERX                      -                                                                 ::
    :: RESET                           -                                                                 ::
    :: SLEEP                           -                                                                 ::
    :: WRITETIMERX                     WRITETIMER(value)                                                 ::
    :: eeprom_read                     unsigned char eeprom_read(unsigned char address);                 ::
    :: eeprom_write                    void eeprom_write(unsigned char address, unsigned char value);    ::
    :: __debug_break                   -                                                                 ::
    :: ___mkstr                        ___mkstr(value)                                                   ::
    :: __EEPROM_DATA                   __EEPROM_DATA(a, b, c, d, e, f, g, h)                             ::
    :: get_cal_data                    double get_cal_data(const unsigned char *);                       ::
    :: _delay                          _delay(n)                                                         ::
    :: _delaywdt                       _delaywdt(n)                                                      ::
    :: _delay3                         _delay3(n)                                                        ::
    :: __builtin_software_breakpoint   void __builtin_software_breakpoint(void);                         ::
    :: __delay_ms                      __delay_ms(time)                                                  ::
    :: __delay_us                      __delay_us(time)                                                  ::
    :: __delaywdt_ms                   __delaywdt_ms(time)                                               ::
    :: __delaywdt_us                   __delaywdt_us(time)                                               ::
    :: __fpnormalize                   double __fpnormalize(double);                                     ::
    :: __osccal_val                    unsigned char __osccal_val(void);                                 ::
    :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    EEPROM_READ

        Synopsis
            #include <xc.h>
            unsigned char eeprom_read(unsigned char address);

        Description
            This function is available for all Mid-range devices that implement EEPROM. For PIC18
            devices, calls to this routine will instead attempt to call the equivalent functions in the
            legacy PIC18 peripheral library, which you must download and install separately. It is
            recommended that for PIC18 devices you use MPLAB MCC to generate EEPROM
            access code, if possible.

            This function tests and waits for any concurrent writes to EEPROM to conclude before
            performing the required operation.

        Example
            #include <xc.h>
            int
            main (void)
            {
                unsigned char serNo;
                serNo = eeprom_read(0x20);
            }

    ___________________________________________________________________________________________

    EEPROM_WRITE

        Synopsis
            #include <xc.h>
            void eeprom_write(unsigned char address, unsigned char value);

        Description
            This function is available for all Mid-range devices that implement EEPROM. For PIC18
            devices, calls to this routine will instead attempt to call the equivalent functions in the
            legacy PIC18 peripheral library, which you must download and install separately. It is
            recommended that for PIC18 devices you use MPLAB MCC to generate EEPROM
            access code, if possible.

            This function tests and waits for any concurrent writes to EEPROM to conclude before
            performing the required operation. The function will initiate the write to EEPROM and
            this process will still be taking place when the function returns. The new data written to
            EEPROM will become valid at a later time. See your device data sheet for exact information
            about EEPROM on your target device.

        Example
            #include <xc.h>
            int
            main (void)
            {
                eeprom_write(0x20, 0x55);
            }

    ___________________________________________________________________________________________

    EEPROM_READ (MACRO)

        Synopsis
            #include <xc.h>
            EEPROM_READ(address);

        Description
            This macro is available for all Mid-range devices that implement EEPROM.
            Unlike the function version, this macro does not wait for any concurrent writes to
            EEPROM to conclude before performing the required operation.

        Example
            #include <xc.h>
            int main (void) {
                unsigned char serNo;
                // wait for end-of-write before EEPROM_READ
                while(WR)
                continue; // read from EEPROM at address
                serNo = EEPROM_READ(0x55);
            }

    ___________________________________________________________________________________________

    EEPROM_WRITE (MACRO)

        Synopsis
            #include <xc.h>
            EEPROM_WRITE(address, value);

        Description
            This macro is available for all Mid-range devices that implement EEPROM.
            This macro tests and waits for any concurrent writes to EEPROM to conclude before
            performing the required operation. The function will initiate the write to EEPROM and
            this process will still be taking place when the function returns. The new data written to
            EEPROM will become valid at a later time. See your device data sheet for exact information
            about EEPROM on your target device.

        Example
            #include <xc.h>
            int main (void) {
                EEPROM_WRITE(0x20, 0x55);
            }

    ___________________________________________________________________________________________

    __BUILTIN_SOFTWARE_BREAKPOINT

        Synopsis
            #include <xc.h>
            void __builtin_software_breakpoint(void);

        Description
            This builtin unconditionally inserts code into the program output which triggers a
            software breakpoint when the code is executed using a debugger.

            The software breakpoint code is only generated for mid-range and PIC18 devices.

            Baseline devices do not support software breakpoints in this way, and the builtin will be
            ignored if used with these devices.

        Example

            #include <xc.h>
            int main (void) {
                __builtin_software_breakpoint(); // stop here to begin
                ...

    ___________________________________________________________________________________________

    __CONDITIONAL_SOFTWARE_BREAKPOINT

        Synopsis
            #include <assert.h>
            __conditional_software_breakpoint(expression)

        Description
            This macro implements a light-weight embedded version of the standard C assert()
            macro, and is used in the same way.

            When executed, the expression argument is evaluated. If the argument is false the
            macro attempts to halt program execution; the macro performs no action if the argument
            is true.

            The macro is removed from the program output if the manifest constant NDEBUG is
            defined. In addition, it is included only for debug builds (i.e., when the __DEBUG macro
            is defined). Thus, it does not consume device resources for production builds.

            If the target device does not support the ability to halt via a software breakpoint, use of
            this macro will trigger a compiler error.

        Example
            #include <assert.h>
            void getValue(int * ip) {
                __conditional_software_breakpoint(ip != NULL);
                ...
            }

    ___________________________________________________________________________________________

    __DEBUG_BREAK

        Synopsis
            #include <xc.h>
            void __debug_break(void);

        Description
            This macro conditionally inserts code into the program output which triggers a software
            breakpoint when the code is executed using a debugger. The code is only generated
            for debug builds (see Section 2.3.7 “What is Different About an MPLAB X IDE Debug
            Build?”) and is omitted for production builds (i.e., when the __DEBUG macro is defined).
            The software breakpoint code is only generated for mid-range and PIC18 devices.
            Baseline devices do not support software breakpoints in this way, and the macro will
            be ignored if used with these devices.

        Example
            #include <xc.h>
            int main (void) {
                __debug_break(); // stop here to begin
                ...
        See also
            __builtin_software_breakpoint()

    ___________________________________________________________________________________________

    __DELAY_MS, __DELAY_US, __DELAYWDT_US, __DELAYWDT_MS

        Synopsis
            __delay_ms(x) // request a delay in milliseconds
            __delay_us(x) // request a delay in microseconds
            __delaywdt_ms(x) // request a delay in milliseconds
            __delaywdt_us(x) // request a delay in microseconds

        Description
            It is often more convenient to request a delay in time-based terms, rather than in cycle
            counts. The macros __delay_ms(x) and __delay_us(x) are provided to meet this
            need. These macros convert the time-based request into instruction cycles that can be
            used with _delay(n). In order to achieve this, these macros require the prior definition
            of preprocessor macro _XTAL_FREQ, which indicates the system frequency. This
            macro should equate to the oscillator frequency (in hertz) used by the system. Note that
            this macro only controls the behavior of these delays and does not affect the device
            execution speed.

            On PIC18 devices only, you can use the alternate WDT-form of these functions, which
            uses the CLRWDT instruction as part of the delay code. See the _delaywdt function.
            The macro argument must be a constant expression. An error will result if these macros
            are used without defining the oscillator frequency symbol, the delay period requested
            is too large, or the delay period is not a constant.

        See also
            _delay(), _delaywdt()

    ___________________________________________________________________________________________

    __EEPROM_DATA

        Synopsis

            #include <xc.h>
            __EEPROM_DATA(a,b,c,d,e,f,g,h)

        Description
            This macro is used to store initial values in the device’s EEPROM registers at the time
            of programming.

            The macro must be given blocks of 8 bytes to write each time it is called, and can be
            called repeatedly to store multiple blocks.

            __EEPROM_DATA() will begin writing to EEPROM address zero, and auto-increments
            the address written to by 8 each time it is used.

        Example
            #include <xc.h>
            __EEPROM_DATA(0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07)
            __EEPROM_DATA(0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F)
            void main (void) {
            }

    ___________________________________________________________________________________________

    __FPNORMALIZE

        Synopsis
            #include <xc.h>
            double __fpnormalize(double fnum)

        Description
            This function can be used to ensure that an arbitrary 32-bit floating-point value (which
            is not the result of a calculation performed by the compiler) conforms to the "relaxed"
            floating-point rules (as described in Section 4.4.4 “Floating-Point Data Types”).
            This function returns the value passed to it, but ensures that any subnormal argument
            is flushed to zero, and converts any negative zero argument to a positive zero result.

        Example
            #include <xc.h>
            void main (void) {
                double input_fp;
                // read in a floating-point value from an external source
                input_fp = getFP();
                // ensure it is formatted using the relaxed rules
                input_fp = __fpnormalize(input_fp);
                ...
            }

    ___________________________________________________________________________________________

    __OSCCAL_VAL

        Synopsis
            #include <xc.h>
            unsigned char __osccal_val(void);

        Description
            This is a pseudo-function that is defined by the code generator to be a label only. The
            label’s value is equated to the address of the retlw instruction, which encapsulates
            the oscillator configuration value. This function is only available for those devices that
            are shipped with such a value stored in program memory.

            Calls to the function will return the device’s oscillator configuration value, which can
            then be used in any expression, if required.

            Note that this function is automatically called by the runtime start-up code (unless you
            have explicitly disabled this option, see Section 3.7.1.14 “osccal”) and you do not need
            to call it to calibrate the internal oscillator.

        Example
            #include <xc.h>
            void main (void) {
                unsigned char c;
                c = __osccal_val();
            }

    ___________________________________________________________________________________________

    _DELAY() , _DELAYWDT

        Synopsis
            #include <xc.h>
            void _delay(unsigned long cycles);
            void _delaywdt(unsigned long cycles);

        Description
            This is an in-line function that is expanded by the code generator. When called, this routine
            expands to an in-line assembly delay sequence. The sequence will consist of code
            that delays for the number of instruction cycles that is specified as the argument. The
            argument must be a constant expression.

            The _delay in-line function can use loops and the NOP instruction to implement the
            delay. The _delaywdt in-line function performs the same task, but will use the
            CLRWDT instruction, as well as loops, to achieve the specified delay.

            An error will result if the requested delay is not a constant expression or is greater than
            50,463,240 instructions. For even larger delays, call this function multiple times.

        Example
            #include <xc.h>
            void main (void) {
                control |= 0x80;
                _delay(10); // delay for 10 cycles
                control &= 0x7F;
            }

    ___________________________________________________________________________________________

    _DELAY3()

        Synopsis
            #include <xc.h>
            void _delay3(unsigned char cycles);

        Description
            This is an in-line function that is expanded by the code generator. It is only available on
            PIC18 and enhanced mid-range devices. When called, this routine expands to an
            in-line assembly delay sequence consisting of code that delays for 3 times the argument
            value, assuming that the argument can be loaded to WREG in one instruction,
            and that there are no errata-workaround NOPs present in the loop. If this is not the
            case, the delay will be longer. The argument can be a byte-sized constant or variable.

        Example
            #include <xc.h>
            void main (void) {
                control |= 0x80;
                _delay3(10); // delay for 30 cycles
                control &= 0x7F;
            }

    ___________________________________________________________________________________________

    ASSERT

        Synopsis
            #include <assert.h>
            void assert (int e)

        Description
            This macro is used for debugging purposes; the basic method of usage is to place
            assertions liberally throughout your code at points where correct operation of the code
            depends upon certain conditions being true initially. An assert() routine can be used
            to ensure at runtime that an assumption holds true. For example, the following
            statement asserts that mode is larger than zero:

                assert(mode > 0);

            If the expression passed to assert() evaluates to false at runtime, the macro
            attempts to print diagnostic information and abort the program. A fuller discussion of
            the uses of assert() is impossible in limited space, but it is closely linked to methods
            of proving program correctness.

            The assert() macro depends on the implementation of the function _fassert().

            The default _fassert() function, built into the library files, first calls the printf()
            function, which prints a message identifying the source file and line number of the
            assertion. Next, _fassert() attempts to terminate program execution by calling
            abort(). The exact behaviour of abort() is dependent on the selected device and
            whether the executable is a debug or production build. For debug builds, abort() will
            consist of a software breakpoint instruction followed by a Reset instruction, if possible.

            For production builds, abort() will consist only of a Reset instruction, if possible. In
            both cases, if a Reset instruction is not available, a goto instruction that jumps to itself
            in an endless loop is output.

            The _fassert() routine can be adjusted to ensure it meets your application needs.
            Include the source file defining this function into your project, if you modify it.

        Example
            #include <assert.h>
            void ptrfunc (struct xyz * tp) {
                assert(tp != 0);
            }

    ___________________________________________________________________________________________

    CLRWDT

        Synopsis
            #include <xc.h>
            CLRWDT();

        Description

            This macro is used to clear the device’s internal watchdog timer.

        Example
            #include <xc.h>
            void main (void) {
                WDTCON=1;
                /* enable the WDT */
                CLRWDT();
            }

    ___________________________________________________________________________________________

    DI, EI

        Synopsis
            #include <xc.h>
            void ei (void)
            void di (void)

        Description
            The di() and ei() routines disable and re-enable interrupts respectively. These are
            implemented as macros. The example shows the use of ei() and di() around access
            to a long variable that is modified during an interrupt. If this was not done, it would be
            possible to return an incorrect value, if the interrupt occurred between accesses to
            successive words of the count value.

            The ei() macro should never be called in an interrupt function.

        Example
            #include <xc.h>
            long count, val;
            void __interrupt(high_priority) tick (void) {
                count++;
            }
            void getticks (void) {
                di();
                val = count;
                ei();
            }

    ___________________________________________________________________________________________

    EVAL_POLY

        Synopsis
            #include <math.h>
            double eval_poly (double x, const double * d, int n)

        Description
            The eval_poly() function evaluates a polynomial, whose coefficients are contained in
            the array d, at x, for example:

                y = x*x*d2 + x*d1 + d0.

            The order of the polynomial is passed in n.

        Example
            #include <stdio.h>
            #include <math.h>
            void main (void) {
                double x, y;
                double d[3] = {1.1, 3.5, 2.7};
                x = 2.2;
                y = eval_poly(x, d, 2);
                printf(“The polynomial evaluated at %f is %f\n”, x, y);
            }

        Return Value
            A double value, being the polynomial evaluated at x.
            abs(), labs()

    ___________________________________________________________________________________________

    GET_CAL_DATA

        Synopsis
            #include <xc.h>
            double get_cal_data (const unsigned char * code_ptr)

        Description
            This function returns the 32-bit floating-point calibration data from the PIC MCU 14000
            calibration space. It cannot be used with other devices. Only use this function to access
            KREF, KBG, VHTHERM and KTC (that is, the 32-bit floating-point parameters). FOSC and
            TWDT can be accessed directly as they are bytes.
        Example
            #include <xc.h>
            void main (void) {
                double x;
                unsigned char y;
                /* Get the slope reference ratio. */
                x = get_cal_data(KREF);
                /* Get the WDT time-out. */
                y = TWDT;
            }

        Return Value
            The value of the calibration parameter

    ___________________________________________________________________________________________

    NOP

        Synopsis
            #include <xc.h>
            NOP();

        Description
            Execute NOP instruction here. This is often useful to fine tune delays or create a handle
            for breakpoints. The NOP instruction is sometimes required during some sensitive
            sequences in hardware.

        Example
            #include <xc.h>
            void crude_delay(void) {
                RA1 = 0;
                NOP();
                RA1 = 1;
            }

    ___________________________________________________________________________________________

    PUTCH

        Synopsis
            #include <conio.h>
            void putch (char c)

        Description
            The putch() function is provided as an empty stub which can be completed as each
            project requires. It must be defined if you intend to use the printf() function. Typically
            this function will accept one byte of data and send this to a peripheral which is
            associated with stdout.

        Example
            #include <conio.h>
            char * x = "This is a string";
            void main (void) {
                char * cp;
                cp = x;
                while(*x) {
                    putch(*x++);
                }
                putch(’\n’);
            }
    ___________________________________________________________________________________________

    READTIMERx

        Synopsis
            #include <xc.h>
            unsigned short READTIMERx (void);

        Description
            The READTIMERx() macro returns the value held by the TMRx register, where x is one
            of the digits 0, 1 or 3.

        Example
            #include <xc>
            void main (void) {
                while(READTIMER0() != 0xFF)
                continue;
                SLEEP();
            }

        See Also
            WRITETIMERx()

        Return Value
            The value held by the TMRx register.

        Note
            This macro can only be used with PIC18 devices.
    ___________________________________________________________________________________________

    RESET

        Synopsis
            #include <xc.h>
            RESET();

        Description
            Execute a RESET instruction here. This will trigger a software device Reset.

        Example
            #include <xc.h>
            void main(void) {
                init();
                while ( ! (fail_code = getStatus())) {
                    process();
                }
                if (fail_code > 2) // something’s serious wrong
                RESET(); // reset the whole device
                // otherwise try restart code from main()
            }
    ___________________________________________________________________________________________

    SLEEP

        Synopsis
            #include <xc.h>
            SLEEP();

        Description
            This macro is used to put the device into a low-power standby mode.

        Example
            #include <xc.h>
            extern void init(void);
            void main (void) {
                init(); /* enable peripherals/interrupts */
                while(1)
                SLEEP(); /* save power while nothing happening */
            }
            WRITETIMERx

        Synopsis
            #include <xc.h>
            void WRITETIMERx (int n);

        Description
            The WRITETIMERx() macro writes the 16-bit argument, n, to both bytes of the TMRx
            register, where x is one of the digits 0, 1 or 3.

        Example
            #include <xc.h>
            void main (void) {
                WRITETIMER1(0x4A);
                while(1) {
                    continue;
                }
            }
        Note
            This macro can only be used with PIC18 devices.


