// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

   125 Hz Main Loop
   +---------------------------------+
   |                                 |
   |                                 | A: Line Sense
   |                                / \
   |                               /   \
   |                              /     \
   |                             /       \ yes
   |                            /  Line   \_________________
   |                            \ Detect? /                 |
   |                             \       /                  |
   |                              \     /                   |
   |                               \   /                    | B: Line in use
   |                                \ / no                 / \
   |                                 |                    /   \
   |                                 |                   /     \ yes
   |                                 |                  / Hold? \_______________________________________
   |                                 | I: Idle|Ring     \       /                                       |
   |                                / \                  \     /                                        |
   |                               /   \                  \   /                                         |
   |                              /     \                  \ / no                                       |
   |                         yes /       \                  |                                           |
   |            ________________/  Ring?  \                 |                                           |
   |           |                \         /                 | F: Active Call                            | C: Currently on HOLD
   |           |                 \       /                 / \                                         / \
   |           |                  \     /                 /   \                                       /   \
   |           |                   \   /                 /     \                                     /     \
   |           |                    \ / no              /       \ yes                               /       \ yes
   |           |                     |                 / A Lead? \________________                 / A Lead? \________________
   |           |                     |                 \         /                |                \         /                |
   |           |                     |                  \       /                 |                 \       /                 |
   |           |                     |                   \     /                  |                  \     /                  |
   |           |                     |                    \   /                   |                   \   / no                |
   |           |                     |                     \ / no                 |                    \ /                    |
   |           |                     |                      |                     |                     |                     |
   |           |                     |                      |                     |                     |                     |
   |           |                     |                      |                     |                     |                     |
   |           | J: Ringing          | K: Idle              | H: HOLD|Hangup      | G: On Call          | E: On HOLD          | D: Pickup from HOLD
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/
   |      Line Ringing           Line Idle         HOLD after 1/20sec        Line In Use           Call On HOLD        Pickup From HOLD    __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |   |  1 -> LX_hold   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |     |-- L1/L2_hold
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/            __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   |  0 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |   |  1 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |   |  1 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |     |-- HOLD_RLY
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/            __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   | RCAD -> RING_RLY|   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |     |-- RING_RLY
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/            __
   |   +-------------------+ +-----------------+  +-------------------+  +-----------------+  +-------------------+  +-----------------+     |
   |   |ring_flash->LX_LAMP| |  0 -> LX_LAMP   |  |hold_flash->LX_LAMP|  |  1 -> LX_LAMP   |  |hold_flash->LX_LAMP|  |  1 -> LX_LAMP   |     |-- L1/L2_LAMP
   |   +-------------------+ +-----------------+  +-------------------+  +-----------------+  +-------------------+  +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/
   |           |                     |                      |                     |                     |                     |
   +-----------o---------------------o----------------------o---------------------o---------------------o---------------------o


INTERLINK SYNC SIGNAL [CPU1]

                                      1 sec
                           |<----------------------->|

    RB6:  _________________       __________.._______       _______..  <-- 5v
                           |     |                   |     |
                           |     |                   |     |
                           |_____|                   |_____|           <-- GND
                            sync                      sync

                        -->|     |<-- One full main loop iteration
                                      Typically 1/250th sec (.004 sec or 4msec)

    RB6 on both cpus are mostly in input mode with weak pullups enabled,
    looking for a ground-going sync signal from the "other" cpu.

    If we haven't received a sync signal yet, when a full second (1000msec)
    has past, "we" switch RB5 to "output" mode and pull the output low,
    and leave it low for a full iteration cycle. During this time we ignore
    signals from the other processor.

    During that time the "other" cpu should see the sync signal, and should
    sync to "us", as they are the slower processor.

    Either cpu can be the "slower" processor, and they may change places
    (depending on heat/environment), but the sync signal will always keep
    the slower processor in sync with the faster one.

    If both processors send sync signals at the same time it's OK;
    neither will see the other's signal because they're /already/ in sync.

    If one processor somehow "misses" the sync signal because the two
    processors are out of step just right, it's OK because they'll drift
    eventually, and sync as soon as they do.

    The sync of the two processors doesn't have to be perfect; it just has
    to be good enough to keep the flashing lights visibly in sync.

FUTURE:

    Make use of the new SECONDARY_DET signal.

    Then we don't need the SYNC signal to keep the two processors synced,
    the PRIMARY can simply be the master source for RING FLASH and HOLD WINK
    and can be the "interrupter" to manage the bell ringing cycle, so that
    ringing is synchronized across all 4 possible lines.

    (The only reason we USED to need the SYNC signal was to keep flashing
    organized, but if the PRIMARY is now the master source for these three
    signals, "syncing" isn't needed)

    With the new design:

        > We sense if we're the PRIMARY or SECONDARY
        > We use the old SYNC line for bidirectional serial data (described below)

    Where:

        If we're the PRIMARY:

            > We send the RING FLASH/HOLD WINK/BELL signals to the SECONDARY (if there is one)
              over the old SYNC pin using bidir serial data packets:
                         ______   __   __   __   _______________________________________
                PRIMARY:       |_|  |_|  |_|  |_|

                                |    |    |    |
                                |    |    |    Hold Wink
                                |    |    Ring Flash
                                |    Bell Ring
                                Sync (start bit)

            > We LISTEN (after sending the above) for any data from the SECONDARY
              indicating if it has any ringing lines, and if so, maintain the
              "6sec Ring Cycle" that tells both boards when to trigger their
              "Ring Relays"

        If we're the SECONDARY:

            > We regularly read the PRIMARY's signal state from the SYNC pin
              into our own variables, and use them

            > After receipt, we send the state of ringing on any of our lines
              back as serial packet, e.g.

                         ______________________________   __   __________
                SECONDARY:                             |_|  |_|

                                                        |    |
                                                        |    |
                                                        |    Line ringing
                                                        Start bit

              When PRIMARY sees secondary's "Line ringing", it starts the 6sec
              master ring cycle (if it hasn't started already), sending the
              "Bell Ring" signal back to the SECONDARY to tell it when to ring
              its line 3/4 "Ring Relays".

    SERIAL DATA:
        The serial data transmission can be done with the SYNC signal configured
        as two cross-connected open collector outputs with WPU (weak pullups) configured,
        so that they ground their outputs briefly to send data.

        Start bits of xmit 0 and 1 can be used for telling the receiver what
        the timing is for '0' and '1'.
        Can either be presence/absence, or the length of the signal, e.g.

        Data0 = 0:
                  _____   _____   __________
                       | |     | |
                       |_|     |_|
                       start   data0
        Data0 = 1:
                  _____   _____      __________
                       | |     |    |
                       |_|     |____|
                       start   data0

        To tell the difference, the 'receiver' just:

            > times the duration of the 'start' bit,

            > compares that to the duration of the data bit(s)

        If the data bit is about double the duration time of the
        start bit (>125%), then it's a '1', otherwise '0'.

    Hold Wink and Ring Flash signals should just run at a constant rate
    from boot of the card. No need to sync flashing with anything else.
                   __ __ __ __ __ __
       HOLD_WINK: |  |  |  |  |  |  |

                    ____       _____
      RING_FLASH: _|    |_____|


    The PRIMARY should be the master for the RING FLASH and WINK FLASH
    signals. Their output would be logically OR'ed with the current
    line state to drive the line lamp, e.g.

         LINE_1_LAMP  = ( LINE_1_HOLD & HOLD_WINK ) | ( LINE_1_RINGING & RING_FLASH )
         LINE_2_LAMP  = ( LINE_1_HOLD & HOLD_WINK ) | ( LINE_1_RINGING & RING_FLASH )

    This way the lamp for a line lights up ONLY if:

         > The line is on Hold and the HOLD_WINK state is on

            - or -

         > The line is Ringing and the RING_FLASH state is on

    The "Ring Cycle" (1 sec bell/3 sec silence) should run on its own timer,
    and can be stopped, and can be started up again AT ANY TIME, so when
    a first call's RING SENSE occurs, the "Ring Cycle" starts immediately,
    so the bell rings on the first ring sense, and then runs the cycle.

    The Ring Cycle will recycle after the 3 second pause IF the 6 second
    timer is still running.

    The 6 second timer is retriggered by the RING SENSE of ANY of the 4 lines.

    When ALL ringing /stops/:

        > The 6 sec timer will run out
        > When the RING CYCLE completes, it simply stops

    Put another way:

        o Each ring sense on ANY ringing line should restart the 6 sec timer
          and set a RING BELL flag for that line. If that line picks up, the
          RING BELL flag for that line should be forced to zero.

        o Each time a Ring Cycle completes, it checks the 6 sec timer:

             > If the 6 sec timer is running, the Ring Cycle replays
             > If the 6 sec timer stopped, the Ring Cycle stops

        o If the ringing line picks up, the ring flag for that line
          should be zeroed. This will either silence ringing, or allow
          ringing to continue if some other line is still actively ringing.

    Each MAIN LOOP ITERATION, the PRIMARY board sends a data word
    to the secondary which it uses for its own interrupter signals
    (Bell ringing, lamp blinking):


             ______   __   __   __   _______________________________________
                   | |  | |  | |  | |
    PRIMARY:       | |  | |  | |  | |
                   |_|  |_|  |_|  |_|

                    |    |    |    |
                    |    |    |    Hold Wink
                    |    |    Ring Flash
                    |    Bell Ring
                    Sync (start bit)

    ..and the SECONDARY can send only one message to the PRIMARY,
    just after the last bit from the PRIMARY, which indicates
    it wants the interrupter MOTOR to keep running:

             ______   __   __   __   _______________________________________
                   | |  | |  | |  | |
    PRIMARY:       | |  | |  | |  | |
                   |_|  |_|  |_|  |_|

                   (see above)

             ______________________________   __   __________
                                           | |  | |
    SECONDARY:                             | |  | |
                                           |_|  |_|

                                            |    |
                                            |    Line Ringing
                                            Start

    When the SECONDARY sends the "Line Ringing" signal,
    the PRIMARY treats this as if one of it's own lines
    has Ring Detect, and starts the 6 second RING SEQUENCE
    timer if it's not already running, and this in turn
    starts the RING CYCLE.

    If after 6 seconds the PRIMARY doesn't see "Line Ringing"
    from the SECONDARY, and its own lines aren't ringing,
    the master "Bell Ring" signal stops being transmitted
    from the PRIMARY.

IDEAS
    If the PRIMARY board is the master, the SECONDARY can simply "hang"
    until it receives data from the PRIMARY, save the data, send a 'reply',
    and then enter the event loop to handle the signals, then repeat (hang
    on wait for the PRIMARY to send new data).

    Data can't be sent too quickly though, otherwise the capacitance
    of the wires will smooth it all out, e.g.

                 1              0              1               1
               ______          ___          ______          ______
       TX:    |      |        |   |        |      |        |      |
           ___|      |________|   |________|      |________|      |_______

                     .                            .               .
                  .   .           .            .   .           .   .
   ACTUAL:      .       .       .  .         .       .       .       .
           ____.          .____.     . _____.           .___.           .___

                                     (shark fins)

    So the signal durations have to be long enough to make these signals
    obvious enough on the read end, including variance in the PIC's
    WPA (Weak Pull Up) resistors, interlink cable length, ambient RF noise,
    etc.

    The minimum data transmissions would need to be one every 1/4 second,
    so perhaps the application loop can be this long. The duration of the
    transmission would need to include keeping an eye on important timing
    inputs (like the A lead release time during a Hold, which has to be
    fairly accurate to the nearest 10th of a second or so)

    So the time domain has to be broken up carefully, either in chunks
    (for non-interrupt multiplexing):


    PRIMARY:    Handle logic, read                           Data xmit
                inputs, debounce                            and receive
              |--------------------------------------------|------------| (repeat)

    ..or for interrupt multiplexing, the DATA TRANSCEIVER handling can happen
    at a much slower and steady rate.

    Perhaps sending bits can be slow enough so that each bit transition
    takes several app loop iters, e.g. transmitting a '1' bit might take
    two iterations just for the 'high' condition, and then an extra full
    app loop just for the low condition, e.g.

                 1              0             1               1
               _______         ___         _______         _______
       TX:    |       |       |   |       |       |       |       |
           ___|       |_______|   |_______|       |_______|       |_______

 APP LOOP: ___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
   ITERS

    ..so just sending 4 bits can take e.g. 16 app loop iters, more if
    the data is all '1's, unless the bit spacing is kept consistent.

    Reading the data probably needs to be interrupt driven to give
    accurate bit timings.

    The SECONDARY should always be in READ MODE until it's received
    a valid transmission from the PRIMARY, and only then can it reply
    with its own data, and the PRIMARY will switch to input mode after
    sending the last bit of its data.

    Whenever a CPU switches to 'input mode', the receiver array should
    be zeroed, and the interrupt should be enabled for any transition
    on that input.

    In single board situations, the PRIMARY simply receives no data
    from the remote, the input held high by its WPU (Weak Pull Up),
    disabling any kind of ring signaling etc.

    This means there must be NO NOISE AT ALL on the interlink input,
    or you'll get phantom ringing for lines #3 and #4!

    The receiver interrupt handler can simply grab snapshots of the hardware
    timer (TMR0) into an array that can later be parsed to compile the bits
    that were received, and these flags can replace the previous read data
    on the next iteration of the event loop, and the xmit/recv process can
    simply be restarted:

            PRIMARY                               SECONDARY
            -------                               ---------
            1. INTS OFF, OUTPUT MODE              1. INTS ON, INPUT MODE
            2. SENDS 8 BITS                       2. READS 8 BITS
            3. INTS ON, SWITCHES TO INPUT MODE    3. INTS OFF, SWITCH TO OUTPUT MODE
            4. READS 8 BITS                       4. SENDS 8 BITS
            (repeat)                              (repeat)

    The current application loop is 125x iters/sec, with hardware timer (TMR1)
    enforcing the time between iters by doing a spin on the timer's value.


    -- TBD --


