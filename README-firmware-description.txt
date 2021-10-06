Contents
========

    1.0 - CPU1: Main KSU Logic
    2.0 - CPU2: Intercom Logic

1.0 - CPU1: Main KSU Logic
--------------------------
The CPU1 PIC handles the main logic for the KSU's operative states, namely:

   - Idle Lines
   - Making a call
   - Ringing/Incoming call/Answering
   - Hold
   - Hangup
   - Picking up a line in use

See README-firmware-logic-diagram.txt for a flow chart of how these
states are managed internally with a state machine.

1.1 - Software Concepts
-----------------------
The software has to manage hardware signals in a way that prevents
false triggers and reliable state transitions.

## I YAM HERE ##

    1.1.1 - Software Debounce - Debounce.h
    --------------------------------------
    Digital inputs can be noisy, so the Debounce.h 'class' (struct + support code) handles
    removing/ignoring "noisy transitions" in following in a modular way:

        > For the input to change from 0-to-1, it must remain in the '1' state for a configured number of samples.
        > For the input to change from 1-to-0, it must remain in the '0' state for a configured number of samples.

    This is used for cleaning up the ring detect input from being a noisy-when-on AC signal
    into a clean squared off signal that remains "on" for the duration of a ring. We also
    debounce the A lead to ignore noise during transitions, or sudden noise introduced over
    long cable runs.

    1.1.2 - Software Timers - TimerMsecs.h
    --------------------------------------
    Often more timers are needed than the PIC chip can provide, so we implement a TimerMsecs.h 'class'
    (struct + support code) that implements an up-counting millisecond timer that is easy to reset,
    update, and detect expiration. This is used

    1.1.3 - KSU Interrupter - Interrupter.h
    ---------------------------------------
    This simulates the Bell System electromechanical "interrupter", which is basically a motor driven
    rotary switch that has several rotary switches, each outputting on-off pulses at different intervals:

         > Ring Relay - 1 sec on 3 sec off (drive the bells by switching the ring generator signal on and off)
         > Ring Flash - 1Hz square wave: 500ms on, 500ms off
         > Hold Flash - 2Hz / 80% duty cycle

    I YAM HERE: More details on how this works over interlink;
                PRIMARY is master, and data received is injected into SECONDARY's
		Interrupter instance, and how SECONDARY tells PRIMARY if calls are
		ringing or on hold to run its interrupter.

    1.1.4 - Bidirectional Data Transmission Using One Shared Conductor
    ------------------------------------------------------------------
    When two boards are interlinked, the single conductor SYNC_ILINK is used to transact data
    between the two boards, to keep their signals synchronized.  See section below entitled
    "BIDIRECTIONAL DATA" for more info.

1.2 - Firmware Overview
-----------------------

The firmware for CPU1 generally does the following:

    > Inputs are monitored for state changes:

        * Ring Detect
        * Line Detect
        * A Lead
        * Primary or Secondary configuration via JP3 and JP4 (SECONDARY_DET)

    > Outputs are changed based on the above input states:

        * Ring Relays
        * Hold Relays
        * Line Lamps
        * Status LED
        * 60Hz buzzer output

    > Handles receiving/transmitting data to interlinked boards (if any) over SYNC_ILINK

    > Handles running the "Interrupter" timed events (ring cadence, lamp flashing)

    > Handles the timers for the 6 second ring cycle(s)

    - Initializes the PIC hardware
    - Initializes the 6 second ring timers for Lines #1 and #2
    - Initializes the "Interrupter" instance
    - Initializes input debounce structs for Lines 1+2 ring detect, and Lines 1+2 A lead detect



   1.1 - Idle
   ----------
   1.2 - Making a call
   -------------------
   1.3 - Ringing/Incoming call/Answering
   -------------------------------------
   1.4 - Hold
   ----------
   1.5 - Hangup
   ------------
   1.6 - Picking up a line in use
   ------------------------------
   

2.0 - CPU2: Intercom Logic
--------------------------


    1.x BIDIRECTIONAL DATA
    ----------------------
    When two boards are interlinked, the single conductor SYNC_ILINK is used to transact
    data between the two boards, to keep their signals synchronized.

    The PIC port is RB6, referred to in the firmware source code as SYNC_ILINK.
    This signal appears on the INTERLINK connector as pin #30, connecting CPU1 RB6 on
    the PRIMARY board to the same chip/port on the SECONDARY board.

    Both boards leave RB6 in "input" mode, with a weak pullup (WPU) enabled for the port.

    At some point the PRIMARY board initiates a transmission by switching RB6 into
    /output/ mode, and sends data one bit at a time by holding the line low for either
    20 cycles (for logic '1') or 10 cycles (for logic '0'). The SECONDARY board has
    IOC (Interrupt On Change) enabled so that when the input goes low, it starts TMR0
    counting, and when it returns to hi, it records TMR0 to determine if the data was a
    '1' or '0'. After receiving the expected data bits (XMIT_BITS), the boards switch
    roles: the SECONDARY then responds with its own data while the PRIMARY listens.

    If TMR0 runs too high waiting for a return transition that never comes, a separate
    interrupt is generated, indicating an error occurred and any data transmitted
    should be ignored.

    In actual operation, the start of each transmission involves sending two start
    bits: a logic '1' followed by a logic '0'. The receiver records the timing of
    these first two bits to use for comparison to the TMR0 values of the subsequent
    data bits. Basically the difference in times between '1' and '0' are compared,
    and the smallest difference is considered the value. e.g.


                                A SINGLE DATA TRANSMISSION
                                ==========================

                                       XMIT_BITS (5)
                       ______________________________________________
                      /                                              \
                           Start Bits                Data Bits
                       ________________       _______________________
                      /                \     /                       \

            __________        _____     _____     _____     _____     _____
                      |      |     |   |     |   |     |   |     |   |
                      |______|     |___|     |_?_|     |_?_|     |_?_|

                         "1"        "0"       0|1       0|1       0|1
                      :      :     :   :
                      :      :     :   :
                      :      :   ->:   :<-- 10 cycles
                      :      :     :   :    (1/2 as long as "1")
                      :      :
                    ->:      :<-- 20 cycles
                      :      :


 * NOTES ON V1.4 AND REV-J4 BOARD
 *
 *       To synchronize both ringing and lamp flashing across both boards,
 *       two major changes:
 *
 *            > Bidirectional data is sent over SYNC_ILINK
 *            > Bell System "Interrupter" class is used to sync ringing/lamps
 *
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
 *
 *       For reliable bidirectional data transmissions, the BUZZ_60HZ signal
 *       is no longer software generated. A hardware PWM is used to generate
 *       the signal, freeing up the software CPU to handle data transceiving..
