// 1A2 "interrupter"
//     Not to be confused with CPU interrupts, this simulates
//     the 1A2 "interrupter", an electro-mechanical rotary switch
//     that generates the on/off switching to drive lamps and ringing.
//
#ifndef INTERRUPTER_H
#define INTERRUPTER_H

// The 1A2 Interrupter struct
//    This simulates a Bell System KSU's electro-mechanical "interrupter".
//
//    0ms     1000ms  2000ms  3000ms  4000ms   <-- up counter from 0 to 4000
//    |.......|.......|.......|.......|
//     _______
//    |       |_______________________   <-- ring_relay
//
//     ___     ___     ___     ___
//    |   |___|   |___|   |___|   |___|  <-- ring_flash
//
//     ___ ___ ___ ___ ___ ___ ___ ___
//    |   |   |   |   |   |   |   |   |  <-- hold_flash
//
//
typedef struct {
    volatile uchar ring_relay;          // runs ring relay (1s on, 3s off)
    volatile uchar ring_flash;          // runs lamp during line ringing (1Hz)
    volatile uchar hold_flash;          // runs lamp during line hold (2Hz)
    uchar stop;                         // stop flag
    TimerMsecs tmr;                     // timer that runs 0 to 4000 (4s cycle)
} Interrupter;

// Initialize the interrupter struct
//     Stops the interrupter timer, outputs all go to zero.
//
void Init_Interrupter(Interrupter *i) {
    i->ring_relay = 0;
    i->ring_flash = 0;
    i->hold_flash = 0;
    i->stop       = 0;
    Init_TimerMsecs(&(i->tmr));
}

// Start the interrupter running (if not already running)
//
//     'msecs' is the number of msecs for the interrupter's cycle,
//     usually 4secs (4000ms)
//
//     This is the equivalent of turning on the 551 KSU "MOTOR" signal,
//     which gets the interrupter's motor turning, generating the
//     different signals.
//
void Start_Interrupter(Interrupter *i, int msecs) {
    i->stop = 0;                                  // clear stop flag (if any)
    if ( IsStopped_TimerMsecs(&(i->tmr)) ) {      // not running?
            Set_TimerMsecs(&i->tmr, msecs);           // start timer running
    }
}

// Stop the interrupter (if not already stopped)
//
//     This is the equivalent of turning OFF the 551 KSU "MOTOR" signal,
//     which LETS THE MOTOR FINISH ITS FULL 4 SEC CYCLE, then stops
//     and turns off the outputs.
//
void Stop_Interrupter(Interrupter *i) {
    i->stop = 1;
}

// Handle updating interrupter's outputs
//
//    This should be called each iter of the main loop (every 'msecs').
//    If interrupter 'started', output variables adjust as needed
//    until stopped. It's up to the line management software to use
//    or ignore these values.
//
void Handle_Interrupter(Interrupter *i, int msecs) {
    int overflow;

    // Int timer not running? early exit
    if ( ! IsRunning_TimerMsecs(&(i->tmr)) ) return;

/** BAD TO REPEAT THIS: MESSES UP SECONDARY DATA INJECTIONS
    // Timer not running? Drive all interrupter output variables OFF..
    if ( ! IsRunning_TimerMsecs(&(i->tmr)) ) {
        i->ring_relay = 0;
        i->ring_flash = 0;
        i->hold_flash = 0;
        i->stop       = 0;
        return;
    }
**/

    // Timer IS running? Drive all interrupter output variables
    overflow = Advance_TimerMsecs(&(i->tmr), msecs);

    // Stop requested?
    //     Don't stop until interrupt timer overflows.
    //     This simulates aspects of the electro-mechanical interrupter.
    //
    if ( i->stop && overflow ) {
        Stop_TimerMsecs(&(i->tmr));    // stop timer
        i->ring_relay = 0;             // all outputs off
        i->ring_flash = 0;
        i->hold_flash = 0;
        i->stop       = 0;             // reset stop flag
        return;
    }

    uint val = Get_TimerMsecs(&(i->tmr));  // counts up: 0 - 4000, repeats
    i->ring_relay = ((val < 1000))       ? 1 : 0;      // 1s on, 3s off
    i->ring_flash = ((val % 1000) < 500) ? 1 : 0;      // 1HZ, 50% duty
    i->hold_flash = ((val % 500 ) < 400) ? 1 : 0;      // 2HZ, 75% duty
}

#endif
