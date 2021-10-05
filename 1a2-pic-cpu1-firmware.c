// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

/* PIC16F features used:
 *     TMR0  - V1.4: interlink data pulse width timing (reset/read during IOC)
 *     TMR1  - V1.4: main loop ITERS_PER_SEC timing (poll)
 *     TMR2  - V1.4: 60Hz PWM (PWM1/CCP1)
 *             (can be TMR2, TMR4 or TMR6; doesn't have to be TMR2)
 *     IOC   - (Interrupt-on-change) - handles receiving interlink data recv
 *     INLVL - Schmitt inputs
 *     WPU   - weak pull-up for most inputs
 */

// TODO:
//     * Check iteration loop speed to make sure it's what it used to be.
//       Try flashing the L1 lamps each iter, and check on scope.
//
//     * Buzzers with TIP125 arrangement seems best run at 120Hz
//       instead of 60 -- 60 sounds too "old and slow".
//
//     * Should investigate if push/pull drivers can replace the TIP125's
//       so that buzzers see full throw +12/GND instead of +12/open.
//
//               #            #
//     #    #   ##            #    #
//     #    #  # #            #    #
//     #    #    #            #    #
//     #    #    #            #######
//      #  #     #      ##         #
//       ##    #####    ##         #
//

// NOTE: For V1.4 to work with REV-J3 and older, you must: CUT THE TRACE FROM CPU2's RA5:
//
//          :
//          |   CPU 2
//          |  +5V     RA5     RA4     RA3
//          |_______________________________ _ _
//            | 1 |   | 2 | # | 3 |   | 4 |
//            |___|   |___| # |___|   |___|
//                      #   #
//                      #   #
//    diagonal          #   #  <-- DONT CUT this nearby trace!
//    trace cut --> \\ #    #
//      here         \\     #
//                   #\\    #         REV-J3 BOARD
//                  #  \\   #          (or older)
//        via --> (O)       #
//                          #
//
//       This prevents SECONDARY_DET from being dragged to ground when CPU2 is
//       powered down (ICM on hook). Not an issue for REV-J4 and up, which has
//       this trace /removed/.
//

/*
 * File:   main.c
 * Author: Greg Ercolano, erco@seriss.com
 * Version: V1.4
 * Current Board Revision: REV-J4
 *
 * Created on Apr 24, 2019, 08:22 AM
 * Compiler: MPLAB X IDE V5.10/5.25/5.50 + XC8 -- Microchip.com
 *
 *     This firmware runs on CPU1 on the 1A2 Multiline Phone Control board,
 *     managing 1A2 functions for Line #1 and #2: Hold, Lamp, and Ringing.
 *
 *     It monitors the Ring Detect and Line Sense inputs for each line,
 *     and responds by controlling the line's lamps, bells, and buzzers.
 *                                   _    _
 *                               V+ | |__| | GND
 *         L1_RING_DET (IN) -- RA5  |      | RA0 -- (OUT) L1_LAMP (DAT)
 *         L1_LINE_DET (IN) -- RA4  |      | RA1 -- (OUT) L1_HOLD_RLY (CLK)
 *            (MCLR) X (IN) -- RA3  |      | RA2 -- (OUT) RING_GEN_POW
 *          L1_A_SENSE (IN) -- RC5  |      | RC0 -- (OUT) L2 HOLD_RLY
 *          L2_A_SENSE (IN) -- RC4  |      | RC1 -- (OUT) BUZZ_RING
 *       SECONDARY_DET (IN) -- RC3  |      | RC2 -- (OUT) L1_RING_RLY
 *         L2_LINE_DET (IN) -- RC6  |      | RB4 -- (OUT) L2_RING_RLY
 *     CPU_STATUS_LED (OUT) -- RC7  |      | RB5 -- (OUT) L2_LAMP
 *         L2_RING_DET (IN) -- RB7  |______| RB6 -- (IN/OUT) SYNC_ILINK
 *
 *                              PIC16F1709 / CPU1
 *                                   REV J4
 *
 *      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *      Copyright (C) 2019, 2021 Seriss Corporation.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *      - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *
 * For the GPL license, see COPYING in the top level directory.
 * For board revisions, see REVISIONS in the top level directory.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * V1.4
 *     * Much changed, this code now depends on REV-J4 board and up.
 *     * Changed INLVLA/B/C from default (TTL?) to Schmitt
 *     * Lock ring cadence between PRIMARY and SECONDARY (using new Interrupter)
 *     * Changed BUZZ_60Hz from software generated to hardware (PWM/CCP/PPS)
 *       (allows bidir send/recv of 8data reliably over SYNC_ILINK)
 *
 * NOTES ON ABOVE:
 *       New "Interrupter" class struct created, emulating the Bell System KSU
 *       interrupter, when enabled, it generates ringing and lamp flashing signals
 *       which are synchronized across interlinked boards.
 *
 *       PRIMARY's running interrupter is "master"; it sends data to SECONDARY
 *       to directly change SECONDARY's internal interrupter variables so that
 *       they're locked in sync. (ring_relay, ring_flash, hold_flash).
 *       The SECONDARY sends data back to the PRIMARY indicating if it wants
 *       the interrupter to start running (e.g. incoming call or line on Hold)
 *       or not (if ringing ends, or all held calls are released)
 *
 *       For this synchronization, we use SYNC_ILINK as bidir data transfer line:
 *           > Transmitter: goes low for either 5 cycles (0) or 10 cycles (1)
 *           > Receiver: Enables IOC (Interrupt On Change) to time pulse width:
 *             >> Low transition: TMR2 is zeroed, and on hi transition, TMR2 is
 *             >> High transition: TMR2 is read to see how long SYNC_ILINK was low
 *
 *       The timing of the signal determines a 1 or 0 bit. Bits are counted in
 *       the variable G_data_index. The first two bits transmitted are always
 *       (1) and (0) which are used as a timing reference for subsequent data bits.
 *
 *       The PRIMARY is always first to send:
 *           PRIMARY sends XMIT_BITS of data to SECONDARY
 *           PRIMARY switches to recv mode
 *           SECONDARY switches to xmit mode, sends XMIT_BITS back to PRIMARY
 *
 *       For this scheme to work, our firmware needs to know if the board it's
 *       running on is configured as PRIMARY or SECONDARY. SECONDARY_DET must
 *       be implemented for this to work. (Note: this works only in REV-J4 and up!)
 *
 *       PRIMARY should xmit first with state of:
 *            > Ring cadence - 1 if ringing, 0 if not
 *            > Ring flash   - 1 if lamp on during ring flash, 0 if lamp off
 *            > Hold flash   - 1 if lamp on during hold flash, 0 if lamp off
 *
 *       SECONDARY should xmit back with state of:
 *            > Any line ringing? (0=no, 1=yes)
 *            > Any line on hold? (0=no, 1=yes)
 *
 *       If any line is ringing or on hold, this should cause PRIMARY to
 *       start its interrupter (if not already).
 */

#define ABS(a)          (((a)<0)?-(a):(a))

//                                                      Port(ABC)
//                                   76543210           |Bit# in port
// Inputs                            ||||||||           ||
#define L1_A_SENSE     ((G_portc & 0b00100000)?0:1) // RC5: low when A lead engaged (0:1 instead of 1:0 to undo negative logic)
#define L2_A_SENSE     ((G_portc & 0b00010000)?0:1) // RC4: low when A lead engaged (0:1 instead of 1:0 to undo negative logic)
#define L1_RING_DET    ((G_porta & 0b00100000)?0:1) // RA5: low on ring detect (0:1 instead of 1:0 to undo negative logic)
#define L2_RING_DET    ((G_portb & 0b10000000)?0:1) // RB7: low on ring detect (0:1 instead of 1:0 to undo negative logic)
#define L1_LINE_DET    ((G_porta & 0b00010000)?0:1) // RA4: low on line detect (0:1 instead of 1:0 to undo negative logic)
#define L2_LINE_DET    ((G_portc & 0b01000000)?0:1) // RC6: low on line detect (0:1 instead of 1:0 to undo negative logic)

// REV-J3 and older: You MUST cut trace from CPU2 RA5 (see above)
// for the following SECONDARY_DET to work. This is because when CPU2 is off,
// it drags SECONDARY_DET low, causing PRIMARY card to think it's secondary.
//
#define SECONDARY_DET  ((G_portc & 0b00001000)?0:1) // RC3: detects if card configured as SECONDARY (JP4)
#define IS_PRIMARY     (SECONDARY_DET) ? 0 : 1      // true if card is PRIMARY
#define IS_SECONDARY   (SECONDARY_DET) ? 1 : 0      // true if card is SECONDARY

// Outputs
#define L1_HOLD_RLY    LATAbits.LATA1               // hi puts L1 on hold
#define L2_HOLD_RLY    LATCbits.LATC0               // hi puts L2 on hold
#define L1_RING_RLY    LATCbits.LATC2               // hi rings L1
#define L2_RING_RLY    LATBbits.LATB4               // hi rings L2
#define RING_GEN_POW   LATAbits.LATA2               // hi supplies +12V to ring generator
#define L1_LAMP        LATAbits.LATA0               // hi turns on L1's lamp on all extensions
#define L2_LAMP        LATBbits.LATB5               // hi turns on L2's lamp on all extensions
#define CPU_STATUS_LED LATCbits.LATC7               // hi turns on CPU STATUS led
#define BUZZ_RING      LATCbits.LATC1               // hi/lo output to buzz phones during incoming calls

// In/Out
#define SYNC_ILINK_OUT LATBbits.LATB6               // RB6[OUT]: pull low when sending sync to "other" cpu
#define SYNC_ILINK_IN  ((PORTBbits.RB6)?1:0)        // RB6[IN]: low when sync sent by "other" cpu

// Ring timers
//     The following macros define the timing of ringing (ring cadence) generated by the KSU.
//     Since the KSU manages multiple lines that can each have their own ring cadence,
//     the KSU regenerates ringing to standardize a single ring cadence across all lines.
//
// Terminology:
//
//     RING_CYCLE_MSECS: The maximum amount of time to wait for the next ring from the CO.
//                       During this time, the line is considered "ringing"; lamps flashing,
//                       bells ringing, and the 12V "ring generator power" turned on.
//                       Each CO ring restarts this timer. There is one timer per line.
//
//       RING_SEQ_MSECS: The 4 sec 1A2 ring sequence: 1 sec ring followed by 3 sec pause.
//                       There is one ring sequence timer for all lines, so that lines
//                       all ring together.
//
// The ring result:
//
//               1sec   2sec   3sec   4sec   5sec   6sec   7sec
//             |......|......|......|......|......|......|.....
//              ______                      ______
//             |      |                    |      |
//             | RING |____________________| RING |_________..
//
//             :                           :
//             :<---- RING_SEQ_MSECS ----->:
//             :                           :

#define RING_CYCLE_MSECS     6000   // #msecs count for ring cycle (how long to keep lamps flashing)
#define RING_SEQ_MSECS       4000   // #msecs count for ring sequence (used for fixed cadence ringing)

// This must be #defined before #includes
#define _XTAL_FREQ 4000000UL        // system oscillator speed in HZ (__delay_ms() needs this)

// --- The following section copy/pasted from MPLAB X menu: Production -> Set Configuration Bits -> Generate Source..
// CONFIG1
#pragma config FOSC     = INTOSC    // USE INTERNAL OSCILLATOR: Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE     = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE    = OFF       // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE    = ON        // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP       = OFF       // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN    = OFF       // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF       // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO     = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN    = OFF       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT     = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = OFF        // Peripheral Pin Select one-way control-> V1.4: Need this *off* to switch between PWM and TRIS
#pragma config ZCDDIS  = ON         // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR)
#pragma config PLLEN   = OFF        // Phase Lock Loop enable (4x PLL is enabled when software sets the SPLLEN bit)
#pragma config STVREN  = ON         // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV    = LO         // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR   = OFF        // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP     = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)
// --- end section

// PIC hardware includes
#include <xc.h>                     // our Microchip C compiler (XC8)
#include <stdint.h>                 // uint8_t, etc.
// DEFINES
#define uchar unsigned char
#define uint  unsigned int

// OUR OWN MODULES
#include "Debounce.h"               // our signal debouncer module
#include "TimerMsecs.h"             // our TimerMsecs module
#include "Interrupter.h"            // our "1A2 interrupter" emulator module

//#include "scopetext.h"            // ERCODEBUG: Displays text on RA0 analog output
                                    // This defines SCOPETEXT_H which we use below..
// REF:
//     TMR0  - V1.4: interlink data pulse width timing (reset/read during IOC)
//     TMR1  - V1.4: main loop ITERS_PER_SEC timing (poll)
//     TMR2  - V1.4: 60Hz PWM (PWM1/CCP1)

#define ITERS_PER_SEC    250        // while() loop iters per second (Hz). *MUST BE EVENLY DIVISIBLE INTO 1000*
#define TIMER1_FREQ      31250      // timer1 counts per second
#define TIMER1_ITER_WAIT (TIMER1_FREQ/ITERS_PER_SEC)
                                    // What Timer1 counts up to every iteration

// GLOBALS
const int  G_msecs_per_iter = (1000/ITERS_PER_SEC);  // #msecs per iter (if ITERS_PER_SEC=125, this is 8. If 250, 4)
TimerMsecs    L1_hold_tmr;             // timer for L1 hold sense
TimerMsecs    L2_hold_tmr;             // timer for L2 hold sense
uchar         L1_hold = 0;             // Line1 HOLD state: 1=call on hold, 0=not on hold
uchar         L2_hold = 0;             // Line2 HOLD state: 1=call on hold, 0=not on hold

// 1A2 Interrupter
//    Handles emulation of a 1A2 KSU "Interrupter", an electro-mechanical device
//    that generates signals for ringing and lamp flashing (for ring flash/hold flash).
//    This entire struct should be volatile, some contents managed by CPU interrupts.
//
Interrupter G_int;
TimerMsecs  L1_ringing_tmr;            // 6sec ring timer reset by each CO ring. Keeps lamps flashing,
TimerMsecs  L2_ringing_tmr;            // 6sec ring timer reset by each CO ring. Keeps lamps flashing,

uchar       G_buzz_signal     = 0;     // 1 indicates isr() should toggle buzzer
int         G_curr_line       = 0;     // "current line" being worked on (1 or 2). Used by HandleLine() and hardware funcs
uchar       G_porta, G_portb, G_portc; // 8 bit input sample buffers, once per main loop iter
uint        G_timer1_cnt      = 0;     // running value of main loop Timer1. counts 0 to TIMER1_FREQ.
uint        G_iter            = 1;     // iteration counter (1-250)

// Interlink Data Xmit/Recv
//     IOC (Interrupt On Change) is used to receive bits, and TMR2 times
//     how long between state changes to determine 1 or 0.
//
//     The first bits sent are 1 followed by 0 to tell the receiver how long
//     to expect 1 and 0 to be.
//
//     Data bits are sent every few main loop iterations to update the
//     various variables managing the remote's state.
//
//     The PRIMARY sends the interrupter state for ring bell, ring flash, hold flash.
//     The SECONDARY sends a bit indicating if the interrupter should be on or not
//     (due to a call on hold or actively ringing lines). This is basically the MOTOR
//     signal in old bell system KSUs that was enabled whenever any line card wanted
//     the interrupter to be generating ringing or lamp flashing.
//
//     The user programs actual ringing across boards using the L1+L2 BELL
//     terminal block, which may or may not be necessary in future revisions
//     if this info is sent as data over the interlink SYNC signal.
//
#define IS_SYNC_POS_EDGE (IOCBFbits.IOCBF6 && PORTBbits.RB6 == 1)  // pos edge interrupt occurred
#define IS_SYNC_NEG_EDGE (IOCBFbits.IOCBF6 && PORTBbits.RB6 == 0)  // neg edge interrupt occurred

#define TIME_1BIT    G_data_times[0]     // tmr0 data pulse width count for a 1 bit
#define TIME_0BIT    G_data_times[1]     // tmr0 data pulse width count for a 0 bit
#define XMIT_BITS    5                   // number of bits PRIMARY/SECONDARY sends to each other (<=8!)

volatile uchar G_data_index = 0;         // data array index
volatile uchar G_data_times[8];          // TMR0 counts for each data bit (indexed by G_data_index)
volatile uchar G_remote_line_ring = 0;   // PRIMARY: 1 if remote (SECONDARY) has a line ringing
volatile uchar G_remote_line_hold = 0;   // PRIMARY: 1 if remote (SECONDARY) has line on hold

////////    #  #    #  #####  ######  #####   #       #  #    #  #    #
////////    #  ##   #    #    #       #    #  #       #  ##   #  #   #
////////    #  # #  #    #    #####   #    #  #       #  # #  #  ####
////////    #  #  # #    #    #       #####   #       #  #  # #  #  #
////////    #  #   ##    #    #       #   #   #       #  #   ##  #   #
////////    #  #    #    #    ######  #    #  ######  #  #    #  #    #
//
//          PRIMARY                 SECONDARY
//          ----------------------- ------------------------------
//       0. DataXmitMode()          (receive mode)
//       1. Send()                       :
//               :                       :
//               :-- send bits ---> parse bits
//               :                       :
//               :-- done --------> HandleRecv()
//
//       2. DataRecvMode();         delay to allow primary to switch to recv mode
//
//       3. (receive mode)          DataXmitMode()
//               :                      :
//       4.      :                  Send()
//               :                      :
//          parse bits <--- send bits --:
//               :                      :
//          HandleRecv() <------ done --:
//                                      :
//                                  DataRecvMode()
//
//       5. Remain in recv mode until next
//          iteration, then goto (0)
//

// TMR0 enable/disable interrupt on overflow
void TMR0IntOnOverflow(int onoff) {
    if ( onoff == 1 ) {
        TMR0              = 0;  // reset timer
        INTCONbits.TMR0IF = 0;  // zero IF
        INTCONbits.TMR0IE = 1;  // enable int on overflow
    } else {
        INTCONbits.TMR0IE = 0;  // disable int on overflow
        INTCONbits.TMR0IF = 0;  // zero IF
    }
}

// Configure hardware to receive data over the SYNC pin.
void DataRecvMode() {
    G_data_index       = 0;              // zero data bit counter
    // SYNC input mode, schmitt, wpu,
    TRISBbits.TRISB6   = 1;              // SYNC_ILINK out (0=out, 1=in)
    INLVLBbits.INLVLB6 = 1;              // TTL(0) vs Schmitt(1) level inputs
    WPUBbits.WPUB6     = 1;              // Enable weak pullup resistor (prevent noise when line open)
    // IOC enable
    INTCONbits.IOCIF   = 0;              // clear IOC interrupt flag before enabling ints
    IOCBPbits.IOCBP6   = 1;              // enable IOC on pos SYNC bit (B6)
    IOCBNbits.IOCBN6   = 1;              // enable IOC on neg SYNC bit (B6)
}

// Configure hardware to send data over SYNC pin.
void DataXmitMode() {
    // TMR0 disable int on overflow
    TMR0IntOnOverflow(0);
    // IOC disable
    IOCBPbits.IOCBP6   = 0;               // disable IOC on pos SYNC bit (B6)
    IOCBNbits.IOCBN6   = 0;               // disable IOC on neg SYNC bit (B6)
    INTCONbits.IOCIF   = 0;               // clear IOC interrupt flag before enabling ints
    // SYNC output mode
    SYNC_ILINK_OUT     = 1;               // resting state for SYNC bit is hi
    TRISBbits.TRISB6   = 0;               // SYNC_ILINK out (0=out, 1=in)
    SYNC_ILINK_OUT     = 1;               // resting state for SYNC bit is hi
}

// Send a single bit to the remote over the interlink
//    Remote will use "interrupt on change" to time edge changes
//
void SendBit(uchar val) {
    // Low time: determined by data being sent
    volatile int count = val ? 20 : 10;
    while (count) { SYNC_ILINK_OUT = 0; --count; }  // stay low for count
    // High time: 3 iters
    count = 20;
    while ( count ) { SYNC_ILINK_OUT = 1; --count; } // stay hi for at least 3
    // Remain hi on exit
}

// Is any local line ringing?
inline int IsAnyLineRinging() {
    return( IsRunning_TimerMsecs(&L1_ringing_tmr) |
            IsRunning_TimerMsecs(&L2_ringing_tmr) );
}

// Is any local line on hold?
inline int IsAnyLineHold() {
    if ( L1_hold || L2_hold ) return 1;
    return 0;
}

// Determine a particular bit that was received to see if it was 1 or 0
volatile uchar ZeroOrOne(uchar val) {
    uchar onediff  = ABS(TIME_1BIT - val);        // how different from known 1 bit time
    uchar zerodiff = ABS(TIME_0BIT - val);        // how different from known 0 bit time
    return ( onediff < zerodiff ) ? 1 : 0;        // closer to 1 bit timing? return 1, else 0
}

// Send data over the interlink.
//    Sends XMIT_BITS total; two start bits for timing, and the rest data.
//
void Send() {
    if ( IS_PRIMARY ) {
        // PRIMARY -> SECONDARY                                                     _
        SendBit(1);                           // 0. start bit (1)                    |
        SendBit(0);                           // 1. start bit (0)                    |__ XMIT_BITS = 5 total
        SendBit(G_int.ring_relay ? 1 : 0);    // 2. is interrupter ringing?          |
        SendBit(G_int.ring_flash ? 1 : 0);    // 3. is interrupter ring flash high?  |
        SendBit(G_int.hold_flash ? 1 : 0);    // 4. is interrupter hold flash high? _|
    } else {
        // SECONDARY -> PRIMARY                                                     _
        SendBit(1);                            // 0. start bit (1)                   |
        SendBit(0);                            // 1. start bit (0)                   |__ XMIT_BITS = 5 total
        SendBit(IsAnyLineRinging() ? 1 : 0);   // 2. any local lines ringing         |
        SendBit(IsAnyLineHold()    ? 1 : 0);   // 3. any local lines on hold         |
        SendBit(0);                            // 4. unused                         _|
    }
}

// Handle receiving data from remote.
//    Main loop calls when XMIT_BITS of data are confirmed received from remote
//    (G_data_index==XMIT_BITS) and it's convenient (timing-wise) to parse data bit timing
//    into actual data. We then reset G_data_index to zero.
//
//
void HandleRecv() {
    if ( G_data_index != XMIT_BITS ) return;    // no complete data yet
    if ( IS_SECONDARY ) {
        // Receive data from PRIMARY
        G_int.ring_relay = ZeroOrOne(G_data_times[2]);
        G_int.ring_flash = ZeroOrOne(G_data_times[3]);
        G_int.hold_flash = ZeroOrOne(G_data_times[4]);
    } else {
        // Receive data from SECONDARY
        G_remote_line_ring = ZeroOrOne(G_data_times[2]);
        G_remote_line_hold = ZeroOrOne(G_data_times[3]);
    }
    G_data_index = 0;
}

////////    #  #    #  #####  ######  #####   #####   #    #  #####   #####     ////////
////////    #  ##   #    #    #       #    #  #    #  #    #  #    #    #       ////////
////////    #  # #  #    #    #####   #    #  #    #  #    #  #    #    #       ////////
////////    #  #  # #    #    #       #####   #####   #    #  #####     #       ////////
////////    #  #   ##    #    #       #   #   #   #   #    #  #         #       ////////
////////    #  #    #    #    ######  #    #  #    #   ####   #         #       ////////

// Interrupt service routine
//     Handles receiving data over the interlink SYNC_ILINK signal.
//     (Both PRIMARY and SECONDARY act as receivers, alternating roles)
//
void __interrupt() isr(void) {
    uint tmr0 = TMR0;                         // save TMR0 value on entry for timing accuracy

    // Handle IOC to receive data bits
    //
    //    XXX: Shouldn't do all this in int handler -- move to main loop!
    //         Set a flag when data received, main loop sees flag at end
    //         of loop and handle it there.
    //
    if ( INTCONbits.IOCIF ) {
        INTCONbits.IOCIF = 0;
        // SYNC_ILINK went lo? Reset TMR0 to zero.
        //
        //    TMR0 is used to time low period to determine if
        //    a logic '0' or logic '1' was sent.
        //
        //    TMR0 also generates an interrupt if it overflows,
        //    an indication transmitter stopped transmitting,
        //    possibly unexpectedly (indicating noise on the line)
        //
        if ( IS_SYNC_NEG_EDGE ) {             // RB6 (SYNC) became negative?
            IOCBFbits.IOCBF6 = 0;             // ack int
            TMR0 = 0;                         // reset TMR0 to time how long SYNC is low
            if ( G_data_index == 0 ) {        // first bit of data?
                TMR0IntOnOverflow(1);         // Start TMR0 int on overflow
            }
        } else if ( IS_SYNC_POS_EDGE ) {      // RB6 (SYNC) became positive?
            IOCBFbits.IOCBF6 = 0;             // ack int
            if ( G_data_index < XMIT_BITS ) {        // receive up to XMIT_BITS of data
                G_data_times[G_data_index++] = tmr0; // save TMR0 timings for Recv() to parse later

                if ( G_data_index == XMIT_BITS ) {   // all bits recvd?
                    TMR0IntOnOverflow(0);            // TMR0 disable overflow ints: all bits recv'd OK
                    if ( IS_SECONDARY ) {            // Secondary turns around with reply
                        HandleRecv();                // we have to handle what we recvd here or we'll loose it
                        DataXmitMode();              // switch to xmit mode
                        __delay_us(800);             // spacing between send/reply bytes
                        Send();                      // send secondary data to primary
                        DataRecvMode();              // return to recv mode
                    } else {
                        // PRIMARY? Remain in recv mode until our next xmit
                        HandleRecv();
                    }
                }
            }
        }
    }

    // TMR0 overflow? Waited too long for data over interlink, reset index..
    if ( INTCONbits.TMR0IF ) {
        INTCONbits.TMR0IF = 0;  // ack int
        G_data_index = 0;       // re-zero index
    }
}

// Turn buzzer 60hz signal on or off
//     Buzzer signal is generated in hardware by PIC's PWM1/CCP1.
//
void Buzz60hz(uchar onoff) {
    if ( onoff == 1 ) {
        // ON: This outputs the PWM's programmed 60Hz output
        RC1PPS = 0x0c;         // enable RC1 -> CCP1 pin assignment
    } else {
        // OFF: Output forced to logic '0' (low)
        RC1PPS = 0x00;         // disable RC1 -> CCP1 pin assignment
        LATCbits.LATC1 = 0;    // make sure RC1 stays /low/ when "off"
    }
}


////////     #   #    #   #   #####     ////////
////////     #   ##   #   #     #       ////////
////////     #   # #  #   #     #       ////////
////////     #   #  # #   #     #       ////////
////////     #   #   ##   #     #       ////////
////////     #   #    #   #     #       ////////

// Initialize TMR0 - Times interlink data pulses
void TMR0_Initialize(void)
{
    // PSA not_assigned; PS 1:2; TMRSE Increment_hi_lo; mask the nWPUEN and INTEDG bits
    OPTION_REGbits.TMR0CS = 0;      // TMR0 Clock Source: 0=use Fosc/4, 1=use T0CKI pin
    OPTION_REGbits.TMR0SE = 0;      // TMR0 Source Edge Select: 1=hi-to-lo on T0CKI, 0=lo-to-hi on T0CKI
    OPTION_REGbits.PSA    = 0;      // Prescaler Assignment bit: 0=use prescaler, 1=don't use prescaler
    OPTION_REGbits.PS     = 0b011;  // Prescaler Select (See pp.245):
                                    //     000=1:2     100=1:32
                                    //     001=1:4     101=1:64
                                    //     010=1:8     110=1:128
                                    //     011=1:16    111=1:256
    // TMR0 255;
    TMR0              = 0x00;
    INTCONbits.TMR0IF = 0;
}

// Used to time main iter loop
void TMR1_Initialize(void)
{
    //T1GSS T1G_pin; TMR1GE disabled; T1GTM disabled; T1GPOL low; T1GGO_nDONE done; T1GSPM disabled;
    T1GCON = 0x00;
    //TMR1H 248;
    TMR1H = 0xF8;
    //TMR1L 16;
    TMR1L = 0x10;
    // Clearing IF flag.
    PIR1bits.TMR1IF = 0;
    // Load the TMR value to reload variable
    // T1CKPS 1:1; T1OSCEN disabled; nT1SYNC synchronize; TMR1CS LFINTOSC; TMR1ON enabled;
    T1CON = 0xC1;
}

void PWM1_Initialize(void)
{
    // PWM1 pin assignment of RC1 -> CCP1/PWM1
    RC1PPS = 0x0C;      //RC1->CCP1:CCP1; 0x0c=0b00001100=Pin(output) Source Selection is CCP1

     // Duty Cycle of 511 (0x1ff)
    //
    // CCP1M="PWM mode"; DC1B=0b01;
    CCP1CON = 0b00111100; //0x3C;
    //          ||||||||       _
    //          ||||||||_ Bit0  |   CCP1M - Mode select bits:
    //          |||||||__ Bit1  |__         1100 - PWM Mode <<<
    //          ||||||___ Bit2  |           1010 - Compare Mode..
    //          |||||____ Bit3 _|           : ..see "27.4 Register Definitions: CCP Control"
    //          ||||           _
    //          ||||_____ Bit4  |__ DC1B  - Duty cycle: lower 2 bits
    //          |||______ Bit5 _|
    //          ||
    //          ||_______ Bit6 x
    //          |________ Bit7 x

    // CCPR1L=127; MSB of Duty Cycle
    //CCPR1L = 0x7F;      // 60HZ Buzz (better for complementary FET's)
    CCPR1L = 0x40;        // 120HZ Buzz (better for TIP125's)
    // CCPR1H 0;
    CCPR1H = 0x00;
    // Selecting Timer 2
    CCPTMRS = 0b00000000; //   _
    //          ||||||||_ Bit0  |__ C1TSEL - CCP1/PWM1 Timer selection bits
    //          |||||||__ Bit1 _|            00=TMR2, 01=TMR4, 10=TMR6
    //          ||||||         _
    //          ||||||___ Bit2  |__ C2TSEL - CCP2/PWM2 Timer selection bits
    //          |||||____ Bit3 _|            00=TMR2, 01=TMR4, 10=TMR6
    //          ||||           _
    //          ||||_____ Bit4  |__ P3TSEL - PWM3 Timer selection bits
    //          |||______ Bit5 _|            00=TMR2, 01=TMR4, 10=TMR6
    //          ||             _
    //          ||_______ Bit6  |__ P4TSEL - PWM4 Timer selection bits
    //          |________ Bit7 _|
}

// TMR2 - Timer used to generate BUZZ60HZ
void TMR2_Initialize(void)
{
    // PWM Period
    //PR2 = 0xff;   // 60HZ Buzz (better for complementary FET's)
    PR2 = 0x81;     // 120HZ (better for TIP125's)
    // TMR2 0;
    TMR2 = 0x00;
    // Clearing IF flag.
    PIR1bits.TMR2IF = 0;
    // T2CON: 26.5 Register Definitions: Timer2 Control pp.259
    //     T2CKPS 1:64; T2OUTPS 1:1; TMR2ON on;
    //
    T2CON = 0b01111111;  //0x7f;
    //        ||||||||        _
    //        ||||||||_ Bit 0  |__ T2CKPS: TMR2 Clock Prescaler Select
    //        |||||||__ Bit 1 _|   11=Prescaler 64, 10=Prescaler 16, 01=Prescaler 4, 00=Prescaler 1
    //        ||||||               ^^^^^^^^^^^^^^^
    //        ||||||___ Bit 2 ---- TMR2 on/off: 0=off, 1=on
    //        |||||           _                        ^^^^
    //        |||||____ Bit 3  |
    //        ||||_____ Bit 4  |__ T2OUTPS: TMR2 Output Postscaler
    //        |||______ Bit 5  |   1111=1:16, 1110=1:15,..0001=1:2, 0000=1:1
    //        ||_______ Bit 6 _|                                    ^^^^^^^^
    //        |
    //        |________ Bit 7
}

// Don't need this as we do this already in PIC_Init()
void OSCILLATOR_Initialize(void)
{
    // SCS FOSC; SPLLEN disabled; IRCF 500KHz_MF;
    // See 6.6 Register Definitions: Oscillator Control
    //
    OSCCON = 0b01101000;  // 0x68;
    //         ||||||||       _
    //         ||||||||_ Bit0  |__ SCS: System Clock Select
    //         |||||||__ Bit1 _|        10=internal osc
    //         ||||||                   01=Secondary
    //         ||||||               >>> 00=Clock determined by FOSC2<2:0> in Config Words
    //         ||||||
    //         ||||||___ Bit2  x
    //         |||||          _
    //         |||||____ Bit3  |   IRCF: Internal Oscillator
    //         ||||_____ Bit4  |__       1111=16MHz HF
    //         |||______ Bit5  |         1110=8MHz or 32MHz HF
    //         ||_______ Bit6 _|         1101=4MHz <<<
    //         |                         :
    //         |                         1010=500kHz HF
    //         |                         :
    //         |                         0111=500kHz MF (Default on RESET)
    //         |                         :
    //         |________ Bit7 SPLLEN: Software PLL Enable

    // SOSCR disabled;
    OSCSTAT = 0x00;
    // TUN 0;
    OSCTUNE = 0x00;
    // SBOREN disabled; BORFS disabled;
    BORCON = 0x00;
}

// IOC Initialize
//    See section "13.0 INTERRUPT-ON-CHANGE" (pp.144)
//    This isn't really necessary, as DataRecvMode() and SetXmitMode() set/unset these as needed
//
void Init_IOC(uchar enable) {
    INTCONbits.IOCIE = 1;   // Interrupt-On-Change Enable: 1=on, 0=off
    INTCONbits.IOCIF = 0;   // Interrupt-On-Change flag: 0=no IOC detected, 1=at least one IOC pin changed state

    if ( enable ) {
        // Enable IOC for SYNC_ILINK (RB6)
        //   We want an interrupt on rising AND falling edges for RB6
        //   so we can time pulse widths to read data over the interlink cable.
        //
        IOCBPbits.IOCBP6 = 1;   // enable IOC for RB6 on Positive Edge
        IOCBNbits.IOCBN6 = 1;   // enable IOC for RB6 on Negative Edge
        IOCBFbits.IOCBF6 = 0;   // clear int flag
    } else {
        IOCBPbits.IOCBP6 = 0;   // disable IOC for RB6 on Positive Edge
        IOCBNbits.IOCBN6 = 0;   // disable IOC for RB6 on Negative Edge
        IOCBFbits.IOCBF6 = 0;   // clear int flag
    }
}

// Initialize PIC chip I/O
//    Configures the clock speed of the processor, which I/O pins are inputs vs outputs,
//    enables input pullup resistors, and enables the interrupt timer.
//
// TTL vs Schmitt input table:
//           ........................
//           : LOW(max) : HIGH(min) :
//           :..........:...........:
//        TTL:   0.8V   :   2.0V    :     Bad zone is 0.9v - 1.9v
//    Schmitt:   0.2V   :   0.8V    :     Bad zone is 0.3v - 0.7v
//           :..........:...........:
//
void Init_PIC() {
    //// CHIP OSCILLATOR SPEED ////
    OSCCONbits.IRCF   = 0b1101;  // 0000=31kHz LF, 0111=500kHz MF (default on reset), 1011=1MHz HF, 1101=4MHz, 1110=8MHz, 1111=16MHz HF
    OSCCONbits.SCS    = 0b10;    // 10=int osc, 00=FOSC determines oscillator
    OSCCONbits.SPLLEN = 0;       // disable 4xPLL (PLLEN in config words must be OFF)
    // OSCILLATOR_Initialize();  // (same as above)

    //// I/O PIN CONFIGURATION ////

    // WPUEN - Weak Pullup ENable
    OPTION_REGbits.nWPUEN = 0;   // Enable WPUEN (weak pullup enable) by clearing bit

    // NOTE: in the following TRISA/B/C data direction registers,
    //       '1' configures an input, '0' configures an output.
    //       'X' indicates a don't care/not implemented on this chip hardware.
    //
    TRISA  = 0b00111000; // data direction for port A (0=output, 1=input)
    INLVLA = 0b00111000; // TTL(0) vs Schmitt(1) level inputs
    WPUA   = 0b00111000; // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ A0 (OUT) L1 LAMP
    //         |||||||__ A1 (OUT) L1 HOLD RLY
    //         ||||||___ A2 (OUT) RING GEN POW
    //         |||||____ A3 (IN)  unused/MCLR
    //         ||||_____ A4 (IN) L1 LINE DET
    //         |||______ A5 (IN) L1 RING DET
    //         ||_______ X
    //         |________ X

    TRISB  = 0b11000000; // data direction for port B (0=output, 1=input)
    INLVLB = 0b11110000; // TTL(0) vs Schmitt(1) level inputs
    WPUB   = 0b11000000; // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ X
    //         |||||||__ X
    //         ||||||___ X
    //         |||||____ X
    //         ||||_____ B4 (OUT) L2 RING RLY
    //         |||______ B5 (OUT) L2 LAMP
    //         ||_______ B6 (IN)  SYNC_ILINK
    //         |________ B7 (IN)  L2 RING DET

    TRISC  = 0b01111000; // data direction for port C (0=output, 1=input)
    INLVLC = 0b01111000; // TTL(0) vs Schmitt(1) level inputs
    WPUC   = 0b01111000; // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ C0 (OUT) L2 HOLD RLY
    //         |||||||__ C1 (OUT) BUZZ 60HZ
    //         ||||||___ C2 (OUT) L1 RING RLY
    //         |||||____ C3 (IN) SECONDARY_DET
    //         ||||_____ C4 (IN)  L2 A SENSE
    //         |||______ C5 (IN)  L1 A SENSE
    //         ||_______ C6 (IN)  L2 LINE DET
    //         |________ C7 (OUT) CPU STATUS

    // Disable analog stuff
    ANSELA = 0x0;
    ANSELB = 0x0;
    ANSELC = 0x0;
    ADCON0 = 0x0;   // disables ADC

    // Disable slew rate controls
    SLRCONA = 0x0;
    SLRCONB = 0x0;
    SLRCONC = 0x0;

    //// HARDWARE MODULE INIT ////
    //   Timers, PWM, CCP, IOS, etc.
    //
    TMR0_Initialize();  // TMR0  - V1.4: times interlink data pulses (reset/read during IOC)
    TMR1_Initialize();  // TMR1  - V1.4: main loop ITERS_PER_SEC timing (poll)
    TMR2_Initialize();  // TMR2  - V1.4: 60Hz PWM (PWM1/CCP1) for BUZZ_60HZ
    PWM1_Initialize();  // PWM1  - V1.4: 60Hz PWM for BUZZ_60HZ
    Init_IOC(0);        // IOC   - V1.4: RB6 generates int on pos AND neg edges during recv mode

    //// INTERRUPTS ////
    INTCONbits.IOCIE = 1;               // IOC (Interrupt-on-change) - for interlink data recv
    INTCONbits.PEIE  = 1;               // PEripheral Interrupt Enable (PEIE)
    INTCONbits.GIE   = 1;               // Global Interrupt Enable (GIE)
    ei();                               // enable interrupts last
}

////////   #####  #    #  #####    ###
////////     #    ##  ##  #    #  #   #
////////     #    # ## #  #    # #     #
////////     #    #    #  #####  #     #
////////     #    #    #  #   #   #   #
////////     #    #    #  #    #   ###

// Set TMR0 interlink data timer to specific value
inline void SetTimer0(uchar val) {
    TMR0 = val;
}

// Return upcounting TMR0 interlink data timer value as a uchar.
//   We use TMR0 to time interlink data bits.
//
inline uchar GetTimer0() {
    return TMR0;
}

////////   #####  #    #  #####     #
////////     #    ##  ##  #    #   ##
////////     #    # ## #  #    #  # #
////////     #    #    #  #####     #
////////     #    #    #  #   #     #
////////     #    #    #  #    #  #####

// Set main loop timer1 to specific value
inline void SetTimer1(uint val) {
    // Stop timer before writing to it
    T1CONbits.TMR1ON = 0;
    // Write new values to TMR1H/TMR1L
    TMR1H = (val >> 8);
    TMR1L = val & 0xff;
    // Start timer again
    T1CONbits.TMR1ON = 1;
}

// Reset main loop timer1 to zero
//    Also resets G_timer1_cnt and G_iter
//
inline void ResetTimer1() {
    SetTimer1(0);
    G_timer1_cnt = 0;     // adjust G_timer1_cnt
    G_iter       = 1;     // adjust iter counter
}

// Return the upcounting TMR1 counter as a 16bit unsigned value
//   (We use TMR1 as a free running timer to time the main loop delay)
//
inline uint GetTimer1() {
    int hi, lo;
    do {
        hi = TMR1H;
        lo = TMR1L;
    } while(hi != TMR1H);     // read 2nd time to check timer for wrap
    return (hi << 8) | lo;
}

// Flash the CPU STATUS led once per second
inline void FlashCpuStatusLED() {
    int count      = G_timer1_cnt % TIMER1_FREQ;
    CPU_STATUS_LED = (count <= 15625) ? 1 : 0;
}

// Change the hardware state of current line's HOLD_RLY
inline void SetHold(char val) {
    switch ( G_curr_line ) {
        case 1: L1_HOLD_RLY = val; L1_hold = val; return;
        case 2: L2_HOLD_RLY = val; L2_hold = val; return;
    }
}

// Change the hardware state of current line's RING_RLY
inline void SetRing(char val) {
    switch ( G_curr_line ) {
        case 1: L1_RING_RLY = val; return;
        case 2: L2_RING_RLY = val; return;
    }
}

// Change the hardware state of current line's LAMP
inline void SetLamp(char val) {
    switch ( G_curr_line ) {
        case 1: L1_LAMP = val; return;
        case 2: L2_LAMP = val; return;
    }
}

// Start the 1/20sec (50msecs) software hold timer value for current line.
inline void StartHoldTimer() {
    switch ( G_curr_line ) {
        case 1: Set_TimerMsecs(&L1_hold_tmr, 50); return;
        case 2: Set_TimerMsecs(&L2_hold_tmr, 50); return;
    }
}

inline void StopHoldTimer() {
    switch ( G_curr_line ) {
        case 1: Stop_TimerMsecs(&L1_hold_tmr); return;
        case 2: Stop_TimerMsecs(&L2_hold_tmr); return;
    }
}

// Is 1/20sec hold timer running?
//    Returns the state of current line's 1/20sec hold timer
//
inline int IsHoldTimer() {
    switch ( G_curr_line ) {
        case 1: return IsRunning_TimerMsecs(&L1_hold_tmr);
        case 2: return IsRunning_TimerMsecs(&L2_hold_tmr);
        default: return 0;   // shouldn't happen
    }
}

// Manage counting one-shot hold timers (if enabled) for current line.
//     Stop timer when it expires.
//
inline void HandleHoldTimer() {
    switch ( G_curr_line ) {
        case 1:
            if ( Advance_TimerMsecs(&L1_hold_tmr, G_msecs_per_iter) ) {
                Stop_TimerMsecs(&L1_hold_tmr);
            }
            return;
        case 2:
            if ( Advance_TimerMsecs(&L2_hold_tmr, G_msecs_per_iter) ) {
                Stop_TimerMsecs(&L2_hold_tmr);
            }
            return;
    }
}

// See if current line's 6sec ring cycle timer is running
inline int IsRingCycle() {
    switch ( G_curr_line ) {
        case 1: return IsRunning_TimerMsecs(&L1_ringing_tmr);
        case 2: return IsRunning_TimerMsecs(&L2_ringing_tmr);
        default: return 0;    // shouldn't happen
    }
}

inline int IsAnyRemoteLineRinging() {
    return G_remote_line_ring;
}

// Start the 4sec (4000msec) software ringing timer value for current line
//      This timer keeps lamp flashing between CO rings.
//      This counts in msec.
//
inline void StartRingingTimer() {
    // See if first ring
    //    If so, reset interrupter so first 1a2 ring happens now.
    //
    if ( IsStopped_TimerMsecs(&L1_ringing_tmr) &&
         IsStopped_TimerMsecs(&L2_ringing_tmr) ) {
        Start_Interrupter(&G_int, RING_SEQ_MSECS);
    }
    // Start 6sec ringing timer running
    switch ( G_curr_line ) {
        case 1: Set_TimerMsecs(&L1_ringing_tmr, RING_CYCLE_MSECS); return;
        case 2: Set_TimerMsecs(&L2_ringing_tmr, RING_CYCLE_MSECS); return;
    }
}

// Stop the 6sec software ringing timer value for current line
//     Also stops the ring sequence timer if no lines are ringing.
//
inline void StopRingingTimer() {
    switch ( G_curr_line ) {
        case 1: Stop_TimerMsecs(&L1_ringing_tmr); break;
        case 2: Stop_TimerMsecs(&L2_ringing_tmr); break;
    }
}

// Advance ringing timers
//     Check for expirations and stop.
//
inline void HandleRingingTimers() {
    // Advance L1 timer if running, and stop if timer expired
    if ( Advance_TimerMsecs(&L1_ringing_tmr, G_msecs_per_iter) ) {
        Stop_TimerMsecs(&L1_ringing_tmr);
    }
    // Advance L2 timer if running, and stop if timer expired
    if ( Advance_TimerMsecs(&L2_ringing_tmr, G_msecs_per_iter) ) {
        Stop_TimerMsecs(&L2_ringing_tmr);
    }
}

// Return the state of the RING_DET optocoupler with noise removed
inline int IsTelcoRinging(Debounce *d) {
    return (d->value > d->thresh) ? 1 : 0;
}

// Initialize debounce struct for ring detect input
//
//     'value' range:
//         max_value     30         __________________
//                                 /
//         on thresh     20 ....../...................
//                               /
//         off thresh    10 ..../.....................
//                       0 ____/
//
//
inline void RingDetectDebounceInit(Debounce *d) {
    d->value      = 0;
    d->max_value  = 30;
    d->on_thresh  = 20;
    d->off_thresh = 10;
    d->thresh     = d->on_thresh;
}

// Manage the L1/L2 RING_DET debounce timers
//     Ignore noise false-triggering RING_DET due to capacitive noise from CO lines
//     during pickup/hangup.
//
// Noisy RING_DET Input:
//                         _      ___   _______________       __          _
//                        | | ||||   | |               | ||| |  || |     | |
//             ___________| |_||||   |_|               |_|||||  ||_|_____| |_______
//                       .           . .               .                 .
//                       .<-- Noise -->.               .<---- Noise ---->.
//                       .           . .               .                 .
//                       .           . .               .                 .
//                       .           . .               .                 .
//  Internal RINGDET COUNTER:        . .  _____________.       _         .
//                       .           ^ . /             \      / \/\_     .
//  on thresh  - - - - - - - - - - -/ \./- - - - - - - -\- - / - - -\ - -.- - - - - on thresh
//                       .         /:  v                 \/\/        \   . _
//  off thresh - - - - - - - - - _/-:- - - - - - - - - - - - - - - - -\ -./ \ - - - off thresh
//             ___________/ \_/\/   :                                 :\_/   \____
//                                  :                                 :
//                                  :<-- hits "on" threshold          :<-- hits "off" threshold
//                                  :                                 :
//  IsTelcoRinging():                _________________________________
//                                  |                                 |
//             _____________________|                                 |_________
//
inline void HandleRingDetTimers(Debounce *d1, Debounce *d2) {
    DebounceNoisyInput(d1, L1_RING_DET);
    DebounceNoisyInput(d2, L2_RING_DET);
}

// Initialize debounce struct for A lead input
//
//     'value' range:
//         max_value     10         __________________
//                                 /
//         on thresh     7  ....../...................
//                               /
//         off thresh    4  ..../.....................
//                       0 ____/
//
//
inline void ALeadDebounceInit(Debounce *d) {
    d->value      = 0;
    d->max_value  = 10;
    d->on_thresh  = 7;
    d->off_thresh = 4;
    d->thresh     = d->on_thresh;
}

inline void HandleALeadDebounce(Debounce *d1, Debounce *d2) {
    DebounceNoisyInput(d1, L1_A_SENSE);
    DebounceNoisyInput(d2, L2_A_SENSE);
}

// Returns 1 if A Lead is engaged
inline int IsALead(Debounce *d) {
    return (d->value > d->thresh) ? 1 : 0;
}

// Return the hardware state of the LINE_DET optocoupler
//     Returns 1 when current is flowing through Tip/Ring, indicating
//     the line is in use; either with an active call, or call on HOLD.
//
inline int IsLineDetect(Debounce *ad) {  // A Lead debounce
    // V1.3D 11/04/2020:
    //      Bypass line detect check IF ringing and no A lead yet.
    //      Line Detect optocoupler can sometimes false trigger during ringing,
    //      apparently detecting ringing as line current, falsely stopping ring cycle.
    //
    if ( IsRingCycle() && !IsALead(ad) ) return 0;  // early exit if noisy line detect

    switch ( G_curr_line ) {
        case 1:  return(L1_LINE_DET ? 1 : 0);
        case 2:  return(L2_LINE_DET ? 1 : 0);
        default: return 0;        // shouldn't happen
    }
}

// Is any remote line on hold?
inline int IsAnyRemoteLineHold() {
    return G_remote_line_hold;
}

// Returns 1 if call is currently on hold, 0 if not
inline int IsHold() {
    switch ( G_curr_line ) {
        case 1:  return(L1_hold); // TODO: try reading hardware state (L1_HOLD_RLY)
        case 2:  return(L2_hold);
        default: return 0;        // shouldn't happen
    }
}

// MANAGE L1/L2 INPUTS/OUTPUTS
//     Set G_curr_line before calling this function to define the line#
//     to be managed during execution.
//
//     In the following, letters in caps (A:, B:, etc) refer to points
//     in the logic diagram file (README-firmware-logic-diagram.txt)
//     included with this .c file.
//
void HandleLine(Debounce *rd, Debounce *ad) {
    // A: Line Detect?
    if ( IsLineDetect(ad) ) {
        // B: Hold?
        if ( IsHold() ) {                      // line currently on HOLD?
            // C: A lead?
            if ( IsALead(ad) ) {               // on hold, but A LEAD now active?
                // D: Pickup From HOLD
                StopHoldTimer();
                StopRingingTimer();
                SetHold(0);                    // return to non-hold state
                SetRing(0);                    // not ringing
                SetLamp(1);                    // line lamp on steady
                return;
            } else {                           // no A lead, still on HOLD
                // E: Call on HOLD
                StopHoldTimer();
                StopRingingTimer();
                SetHold(1);                    // stay on hold
                SetRing(0);                    // not ringing
                SetLamp(G_int.hold_flash);     // flash line lamp at HOLD rate
                return;
            }
        } else {
            // F: Active Call
            if ( IsALead(ad) ) {
                // G: On Call -- Line in use
                StopHoldTimer();
                StopRingingTimer();
                SetHold(0);                    // not on hold
                SetRing(0);                    // not ringing
                SetLamp(1);                    // lamp on steady
                return;
            } else {
                // H: Hold or Hangup
                //    Wait 1/20sec: if A lead still gone but Line Det active, call went on HOLD.
                //    Otherwise, if Line Det dropped too, call was hung up.
                //
                if ( ! IsHoldTimer() ) {       // A LEAD /just/ dropped?
                    StartHoldTimer();          // Start 1/20sec hold timer
                    StopRingingTimer();
                    SetHold(0);                // no HOLD yet until verified when 1/20th timer expires
                    SetRing(0);                // not ringing
                    SetLamp(1);                // lamp on steady; still active call, not sure if HOLD yet
                    return;
                }

                // Count hold timer
                HandleHoldTimer();

                // Watch for hold condition when timer expires
                if ( IsHoldTimer() ) {         // HOLD timer still running?
                    //HandleHoldTimer();       // timer handled in main()
                    StopRingingTimer();
                    SetHold(0);                // no HOLD relay yet until verified
                    SetRing(0);                // not ringing
                    SetLamp(1);                // lamp on steady; still active call, not sure if HOLD yet
                    return;
                } else {                       // HOLD timer expired: if we're still here, put call on HOLD.
                    StopHoldTimer();
                    StopRingingTimer();
                    SetHold(1);                // put call on hold
                    SetRing(0);                // not ringing
                    SetLamp(G_int.hold_flash); // flash line lamp at HOLD rate
                    return;
                }
            }
        }
    } else {
        // I: Idle (no line detect)
        if ( IsTelcoRinging(rd) ) {
            // CO is currently ringing the line?
            // Restart 'line ringing' timer whenever a ring is detected.
            //
            StartRingingTimer();               // Start 6sec ring timer
        }
        // Line has incoming call, either ringing or between rings.
        // Keep lamp blinking between rings, have 1A2 ring relay
        // follow the CO's ring cadence.
        //
        if ( IsRingCycle() ) {
            // J: Line is ringing
            //    Let 6sec ringing counter count to keep lamp flashing between rings,
            //    and let ring relay follow the CO's ringing signal for same cadence.
            //
            StopHoldTimer();
            SetHold(0);
            SetRing(G_int.ring_relay);         // manage ring relay
            SetLamp(G_int.ring_flash);         // flash line lamp at RING rate
            return;
        } else {
            // K: Line idle
            //    Nothing is going on; no ringing, no hold, no call, nuthin.
            //
            StopHoldTimer();
            StopRingingTimer();
            SetHold(0);                        // disable HOLD relay
            SetRing(0);                        // disable ringing
            SetLamp(IsALead(ad) ? 1 : 0);      // Keep lamp lit if A LEAD (prevent flash during rotary)
            return;
        }
    }
}

// Buffer the hardware state of PIC's PORTA/B/C all at once.
//     Run this at the beginning of each iter of the 125Hz main loop.
//     We then do bit tests on these buffered values, to avoid multiple
//     hardware reads throughout execution to avoid sampling parallax.
//
inline void SampleInputs() {
    // Buffer the hardware input states
    G_porta = PORTA;
    G_portb = PORTB;
    G_portc = PORTC;
}

// Drive buzzer's PWM 60hz signal based on state of interrupter's ring_relay
//     This signal is already gated by the ring relay's contacts,
//     but lets turn it off when not needed, just to prevent
//     60hz switching noise as background radiation on the board..
//
inline void HandleBuzzRing() {
    Buzz60hz(G_int.ring_relay ? 1 : 0);
}

// Handle running or stopping the interrupter
void HandleInterrupter() {
    // Reasons the interrupter should be running..
    if ( IS_PRIMARY ) {
        // PRIMARY? Run the interrupter
        if ( IsAnyLineRinging() || IsAnyRemoteLineRinging() ||  // local or interlink ringing?
             IsAnyLineHold()    || IsAnyRemoteLineHold() ) {    // local or interlink on hold?
            Start_Interrupter(&G_int, RING_SEQ_MSECS);
        } else {
            Stop_Interrupter(&G_int);
        }
        Handle_Interrupter(&G_int, G_msecs_per_iter);
    } else {
        // SECONDARY? Do NOT run the interrupter; PRIMARY tells us what to do.
        Stop_Interrupter(&G_int); // Stops interrupter in case it's running (zeroes data, but only once)
                                  // If it's not running, does nothing.
    }
}

#ifdef SCOPETEXT_H
// Print some debugging data to scope on RA0 analog output
void PrintDebugData() {
    char s[40];
    char *p = s;
    *p++ = '0';
    *p++ = '=';
    p = SCOPETEXT_AsHex(TIME_0BIT, p);
    *p++ = ',';
    *p++ = '1';
    *p++ = '=';
    p = SCOPETEXT_AsHex(TIME_1BIT, p);
    SCOPETEXT_Print(s);
}
#endif // SCOPETEXT_H

//////   #    #    ##     #   #    #
//////   ##  ##   #  #    #   ##   #
//////   # ## #  #    #   #   # #  #
//////   #    #  ######   #   #  # #
//////   #    #  #    #   #   #   ##
//////   #    #  #    #   #   #    #
//
// Main -- Initializes hardware, and enters main while() loop.
//
//         The timing of the while() loop is locked to the hardware TIMER1
//         to ensure consistent timing between iterations, regardless of
//         the execution time of while() loop code.
//
void main(void) {
    uint timer1_wait = 0;

    // Ring detect debounce/hysteresis struct
    Debounce ringdet_d1, ringdet_d2;

    // A lead debounce/hysteresis struct
    Debounce a_lead_d1, a_lead_d2;

    // Initialize PIC chip
    Init_PIC();

    // Initialize L1/L2 ring timers
    Init_TimerMsecs(&L1_ringing_tmr);
    Init_TimerMsecs(&L2_ringing_tmr);

    // Initialize interrupter
    Init_Interrupter(&G_int);

    RingDetectDebounceInit(&ringdet_d1);
    RingDetectDebounceInit(&ringdet_d2);
    ALeadDebounceInit(&a_lead_d1);
    ALeadDebounceInit(&a_lead_d2);

    // Start hardware timer1 at zero
    ResetTimer1();

    // Both primary and secondary start in recv mode
    DataRecvMode();

#ifdef SCOPETEXT_H
    // FONTSCOPE DEBUGGING: ERCODEBUG
    //    Configure RA0 as an analog output so we can use it to display text
    //    with the fontscope module.
    //
    {
        // DAC initialize
        // See pp.238 "22.6 Register Definitions: DAC control"
        DAC1CON0             = 0x00;  // start with zero, then set bits we want
        DAC1CON0bits.DAC1EN  = 1;     // enable DAC1
        DAC1CON0bits.DAC1OE1 = 1;     // enable output to DAC1OUT1 pin (RA0)
        DAC1CON0bits.DAC1PSS = 0;     // use VDD for analog pos vref
        DAC1CON0bits.DAC1NSS = 0;     // use VSS for analog neg vref
        DAC1CON1             = 0x00;  // initial output voltage gnd
    }
#endif //SCOPETEXT_H

    // Loop at ITERS_PER_SEC
    //     If ITERS_PER_SEC is 125, this is an 8msec loop
    //
    while (1) {
        // DO THIS FIRST!
        //    Sample input ports all at once to prevent races and "impossible"
        //    state combos that can happen with serialized polling..
        //
        SampleInputs();

        //// NEW_SYNC ////
        // Take snapshot of timer1's count, reset timer if reaches 1sec count.
        //    G_timer1_cnt is the time base for all flashing, ringing, etc.
        //
        G_timer1_cnt = GetTimer1();             // runs 0 to TIMER1_FREQ (31250)
        if ( G_timer1_cnt >= TIMER1_FREQ ) {    // wrap?
            ResetTimer1();                      // reset hardware timer to zero
            G_timer1_cnt = 0;                   // snapshot zero count
            G_iter       = 1;                   // reset iter counter to zero
        }

        // Determine timer count to wait for
        timer1_wait = G_iter * 125;             // 125*250=31250

        // Keep CPU STATUS lamp flashing
        FlashCpuStatusLED();

        // Manage the A lead inputs
        HandleALeadDebounce(&a_lead_d1, &a_lead_d2);

        // Manage counting the 1/10sec L1/L2_ringdet_timer each iter.
        HandleRingDetTimers(&ringdet_d1, &ringdet_d2);

        // Manage the 1A2 interrupter struct
        HandleInterrupter();

        // Manage counting the ring cycle related counters
        HandleRingingTimers();

        // Manage buzz ringing
        HandleBuzzRing();

        // Handle logic signals for Line #1 and Line #2
        G_curr_line = 1; HandleLine(&ringdet_d1, &a_lead_d1);
        G_curr_line = 2; HandleLine(&ringdet_d2, &a_lead_d2);

        // Handle ring generator power
        //     This should be 'on' during and between all ringing on L1 or L2,
        //     so we combine the 6sec ringing timers for both lines..
        //
        RING_GEN_POW = IsAnyLineRinging();

        // PRIMARY? SEND DATA TO REMOTE EVERY 10 ITERS
        if ( IS_PRIMARY ) {
            if ( (G_iter % 5) == 0 ) {
                DataXmitMode();  // switch to xmit mode
                Send();          // send data to secondary
                DataRecvMode();  // immediately switch to recv mode, expect secondary to reply
            }
        }

#ifdef SCOPETEXT_H
        if ( G_iter == 1 ) PrintDebugData();           // ERCODEBUG
#endif //SCOPETEXT_H

        // LOOP DELAY: Wait on hardware timer for iteration delay
        //             This gives accurate main loop iters, no matter speed of code execution
        //
        while ( GetTimer1() < timer1_wait ) { }        // wait until timer reaches iter timer count

        // Advance iteration timer
        ++G_iter;                                      // may go above 250! (we don't want to wrap until timer resets)
    }
}
