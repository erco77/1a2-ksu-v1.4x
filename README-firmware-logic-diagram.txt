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
   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |   |  1 -> LX_hold   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |   |  0 -> LX_hold   |     |-- L1/L2_hold state
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/           __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   |  0 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |   |  1 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |   |  1 -> HOLD_RLY  |   |  0 -> HOLD_RLY  |     |-- HOLD_RLY
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/           __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   | RCAD -> RING_RLY|   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |   |  0 -> RING_RLY  |     |-- RING_RLY
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/           __
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+     |
   |   |   LF -> LAMP    |   |  0 -> LX_LAMP   |   |    LW -> LAMP   |   |  1 -> LX_LAMP   |   |    LW -> LAMP   |   |  1 -> LX_LAMP   |     |-- L1/L2_LAMP
   |   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   +-----------------+   __|
   |           |                     |                      |                     |                     |                     |
   |          \|/                   \|/                    \|/                   \|/                   \|/                   \|/
   |           |                     |                      |                     |                     |                     |
   +-----------o---------------------o----------------------o---------------------o---------------------o---------------------o

   In the above:

        RCAD = Ring Cadence: "1sec on, 3sec off", signal provided by "Interrupter"
          LF = Lamp Flash: 1Hz flash during incoming call, signal provided by "Interrupter"
          LW = Lamp Wink:  2Hz/80% Duty during call on hold, signal provided by "Interrupter"

- OLD - INTERLINK SYNC SIGNAL [CPU1]
- OLD - 
- OLD -                                       1 sec
- OLD -                            |<----------------------->|
- OLD - 
- OLD -     RB6:  _________________       __________.._______       _______..  <-- 5v
- OLD -                            |     |                   |     |
- OLD -                            |     |                   |     |
- OLD -                            |_____|                   |_____|           <-- GND
- OLD -                             sync                      sync
- OLD - 
- OLD -                         -->|     |<-- One full main loop iteration
- OLD -                                       Typically 1/250th sec (.004 sec or 4msec)
- OLD - 
- OLD -     RB6 on both cpus are mostly in input mode with weak pullups enabled,
- OLD -     looking for a ground-going sync signal from the "other" cpu.
- OLD - 
- OLD -     If we haven't received a sync signal yet, when a full second (1000msec)
- OLD -     has past, "we" switch RB5 to "output" mode and pull the output low,
- OLD -     and leave it low for a full iteration cycle. During this time we ignore
- OLD -     signals from the other processor.
- OLD - 
- OLD -     During that time the "other" cpu should see the sync signal, and should
- OLD -     sync to "us", as they are the slower processor.
- OLD - 
- OLD -     Either cpu can be the "slower" processor, and they may change places
- OLD -     (depending on heat/environment), but the sync signal will always keep
- OLD -     the slower processor in sync with the faster one.
- OLD - 
- OLD -     If both processors send sync signals at the same time it's OK;
- OLD -     neither will see the other's signal because they're /already/ in sync.
- OLD - 
- OLD -     If one processor somehow "misses" the sync signal because the two
- OLD -     processors are out of step just right, it's OK because they'll drift
- OLD -     eventually, and sync as soon as they do.
- OLD - 
- OLD -     The sync of the two processors doesn't have to be perfect; it just has
- OLD -     to be good enough to keep the flashing lights visibly in sync.
- OLD - 
- OLD - FUTURE:
- OLD - 
- OLD -     Make use of the new SECONDARY_DET signal.
- OLD - 
- OLD -     Then we don't need the SYNC signal to keep the two processors synced,
- OLD -     the PRIMARY can simply be the master source for RING FLASH and HOLD WINK
- OLD -     and can be the "interrupter" to manage the bell ringing cycle, so that
- OLD -     ringing is synchronized across all 4 possible lines.
- OLD - 
- OLD -     (The only reason we USED to need the SYNC signal was to keep flashing
- OLD -     organized, but if the PRIMARY is now the master source for these three
- OLD -     signals, "syncing" isn't needed)
- OLD - 
- OLD -     With the new design:
- OLD - 
- OLD -         > We sense if we're the PRIMARY or SECONDARY
- OLD -         > We use the old SYNC line for bidirectional serial data (described below)
- OLD - 
- OLD -     Where:
- OLD - 
- OLD -         If we're the PRIMARY:
- OLD - 
- OLD -             > We send the RING FLASH/HOLD WINK/BELL signals to the SECONDARY (if there is one)
- OLD -               over the old SYNC pin using bidir serial data packets:
- OLD -                          ______   __   __   __   _______________________________________
- OLD -                 PRIMARY:       |_|  |_|  |_|  |_|
- OLD - 
- OLD -                                 |    |    |    |
- OLD -                                 |    |    |    Hold Wink
- OLD -                                 |    |    Ring Flash
- OLD -                                 |    Bell Ring
- OLD -                                 Sync (start bit)
- OLD - 
- OLD -             > We LISTEN (after sending the above) for any data from the SECONDARY
- OLD -               indicating if it has any ringing lines, and if so, maintain the
- OLD -               "6sec Ring Cycle" that tells both boards when to trigger their
- OLD -               "Ring Relays"
- OLD - 
- OLD -         If we're the SECONDARY:
- OLD - 
- OLD -             > We regularly read the PRIMARY's signal state from the SYNC pin
- OLD -               into our own variables, and use them
- OLD - 
- OLD -             > After receipt, we send the state of ringing on any of our lines
- OLD -               back as serial packet, e.g.
- OLD - 
- OLD -                          ______________________________   __   __________
- OLD -                 SECONDARY:                             |_|  |_|
- OLD - 
- OLD -                                                         |    |
- OLD -                                                         |    |
- OLD -                                                         |    Line ringing
- OLD -                                                         Start bit
- OLD - 
- OLD -               When PRIMARY sees secondary's "Line ringing", it starts the 6sec
- OLD -               master ring cycle (if it hasn't started already), sending the
- OLD -               "Bell Ring" signal back to the SECONDARY to tell it when to ring
- OLD -               its line 3/4 "Ring Relays".
- OLD - 
- OLD -     SERIAL DATA:
- OLD -         The serial data transmission can be done with the SYNC signal configured
- OLD -         as two cross-connected open collector outputs with WPU (weak pullups) configured,
- OLD -         so that they ground their outputs briefly to send data.
- OLD - 
- OLD -         Start bits of xmit 0 and 1 can be used for telling the receiver what
- OLD -         the timing is for '0' and '1'.
- OLD -         Can either be presence/absence, or the length of the signal, e.g.
- OLD - 
- OLD -         Data0 = 0:
- OLD -                   _____   _____   __________
- OLD -                        | |     | |
- OLD -                        |_|     |_|
- OLD -                        start   data0
- OLD -         Data0 = 1:
- OLD -                   _____   _____      __________
- OLD -                        | |     |    |
- OLD -                        |_|     |____|
- OLD -                        start   data0
- OLD - 
- OLD -         To tell the difference, the 'receiver' just:
- OLD - 
- OLD -             > times the duration of the 'start' bit,
- OLD - 
- OLD -             > compares that to the duration of the data bit(s)
- OLD - 
- OLD -         If the data bit is about double the duration time of the
- OLD -         start bit (>125%), then it's a '1', otherwise '0'.
- OLD - 
- OLD -     Hold Wink and Ring Flash signals should just run at a constant rate
- OLD -     from boot of the card. No need to sync flashing with anything else.
- OLD -                    __ __ __ __ __ __
- OLD -        HOLD_WINK: |  |  |  |  |  |  |
- OLD - 
- OLD -                     ____       _____
- OLD -       RING_FLASH: _|    |_____|
- OLD - 
- OLD - 
- OLD -     The PRIMARY should be the master for the RING FLASH and WINK FLASH
- OLD -     signals. Their output would be logically OR'ed with the current
- OLD -     line state to drive the line lamp, e.g.
- OLD - 
- OLD -          LINE_1_LAMP  = ( LINE_1_HOLD & HOLD_WINK ) | ( LINE_1_RINGING & RING_FLASH )
- OLD -          LINE_2_LAMP  = ( LINE_1_HOLD & HOLD_WINK ) | ( LINE_1_RINGING & RING_FLASH )
- OLD - 
- OLD -     This way the lamp for a line lights up ONLY if:
- OLD - 
- OLD -          > The line is on Hold and the HOLD_WINK state is on
- OLD - 
- OLD -             - or -
- OLD - 
- OLD -          > The line is Ringing and the RING_FLASH state is on
- OLD - 
- OLD -     The "Ring Cycle" (1 sec bell/3 sec silence) should run on its own timer,
- OLD -     and can be stopped, and can be started up again AT ANY TIME, so when
- OLD -     a first call's RING SENSE occurs, the "Ring Cycle" starts immediately,
- OLD -     so the bell rings on the first ring sense, and then runs the cycle.
- OLD - 
- OLD -     The Ring Cycle will recycle after the 3 second pause IF the 6 second
- OLD -     timer is still running.
- OLD - 
- OLD -     The 6 second timer is retriggered by the RING SENSE of ANY of the 4 lines.
- OLD - 
- OLD -     When ALL ringing /stops/:
- OLD - 
- OLD -         > The 6 sec timer will run out
- OLD -         > When the RING CYCLE completes, it simply stops
- OLD - 
- OLD -     Put another way:
- OLD - 
- OLD -         o Each ring sense on ANY ringing line should restart the 6 sec timer
- OLD -           and set a RING BELL flag for that line. If that line picks up, the
- OLD -           RING BELL flag for that line should be forced to zero.
- OLD - 
- OLD -         o Each time a Ring Cycle completes, it checks the 6 sec timer:
- OLD - 
- OLD -              > If the 6 sec timer is running, the Ring Cycle replays
- OLD -              > If the 6 sec timer stopped, the Ring Cycle stops
- OLD - 
- OLD -         o If the ringing line picks up, the ring flag for that line
- OLD -           should be zeroed. This will either silence ringing, or allow
- OLD -           ringing to continue if some other line is still actively ringing.
- OLD - 
- OLD -     Each MAIN LOOP ITERATION, the PRIMARY board sends a data word
- OLD -     to the secondary which it uses for its own interrupter signals
- OLD -     (Bell ringing, lamp blinking):
- OLD - 
- OLD - 
- OLD -              ______   __   __   __   _______________________________________
- OLD -                    | |  | |  | |  | |
- OLD -     PRIMARY:       | |  | |  | |  | |
- OLD -                    |_|  |_|  |_|  |_|
- OLD - 
- OLD -                     |    |    |    |
- OLD -                     |    |    |    Hold Wink
- OLD -                     |    |    Ring Flash
- OLD -                     |    Bell Ring
- OLD -                     Sync (start bit)
- OLD - 
- OLD -     ..and the SECONDARY can send only one message to the PRIMARY,
- OLD -     just after the last bit from the PRIMARY, which indicates
- OLD -     it wants the interrupter MOTOR to keep running:
- OLD - 
- OLD -              ______   __   __   __   _______________________________________
- OLD -                    | |  | |  | |  | |
- OLD -     PRIMARY:       | |  | |  | |  | |
- OLD -                    |_|  |_|  |_|  |_|
- OLD - 
- OLD -                    (see above)
- OLD - 
- OLD -              ______________________________   __   __________
- OLD -                                            | |  | |
- OLD -     SECONDARY:                             | |  | |
- OLD -                                            |_|  |_|
- OLD - 
- OLD -                                             |    |
- OLD -                                             |    Line Ringing
- OLD -                                             Start
- OLD - 
- OLD -     When the SECONDARY sends the "Line Ringing" signal,
- OLD -     the PRIMARY treats this as if one of it's own lines
- OLD -     has Ring Detect, and starts the 6 second RING SEQUENCE
- OLD -     timer if it's not already running, and this in turn
- OLD -     starts the RING CYCLE.
- OLD - 
- OLD -     If after 6 seconds the PRIMARY doesn't see "Line Ringing"
- OLD -     from the SECONDARY, and its own lines aren't ringing,
- OLD -     the master "Bell Ring" signal stops being transmitted
- OLD -     from the PRIMARY.
- OLD - 
- OLD - IDEAS
- OLD -     If the PRIMARY board is the master, the SECONDARY can simply "hang"
- OLD -     until it receives data from the PRIMARY, save the data, send a 'reply',
- OLD -     and then enter the event loop to handle the signals, then repeat (hang
- OLD -     on wait for the PRIMARY to send new data).
- OLD - 
- OLD -     Data can't be sent too quickly though, otherwise the capacitance
- OLD -     of the wires will smooth it all out, e.g.
- OLD - 
- OLD -                  1              0              1               1
- OLD -                ______          ___          ______          ______
- OLD -        TX:    |      |        |   |        |      |        |      |
- OLD -            ___|      |________|   |________|      |________|      |_______
- OLD - 
- OLD -                      .                            .               .
- OLD -                   .   .           .            .   .           .   .
- OLD -    ACTUAL:      .       .       .  .         .       .       .       .
- OLD -            ____.          .____.     . _____.           .___.           .___
- OLD - 
- OLD -                                      (shark fins)
- OLD - 
- OLD -     So the signal durations have to be long enough to make these signals
- OLD -     obvious enough on the read end, including variance in the PIC's
- OLD -     WPA (Weak Pull Up) resistors, interlink cable length, ambient RF noise,
- OLD -     etc.
- OLD - 
- OLD -     The minimum data transmissions would need to be one every 1/4 second,
- OLD -     so perhaps the application loop can be this long. The duration of the
- OLD -     transmission would need to include keeping an eye on important timing
- OLD -     inputs (like the A lead release time during a Hold, which has to be
- OLD -     fairly accurate to the nearest 10th of a second or so)
- OLD - 
- OLD -     So the time domain has to be broken up carefully, either in chunks
- OLD -     (for non-interrupt multiplexing):
- OLD - 
- OLD - 
- OLD -     PRIMARY:    Handle logic, read                           Data xmit
- OLD -                 inputs, debounce                            and receive
- OLD -               |--------------------------------------------|------------| (repeat)
- OLD - 
- OLD -     ..or for interrupt multiplexing, the DATA TRANSCEIVER handling can happen
- OLD -     at a much slower and steady rate.
- OLD - 
- OLD -     Perhaps sending bits can be slow enough so that each bit transition
- OLD -     takes several app loop iters, e.g. transmitting a '1' bit might take
- OLD -     two iterations just for the 'high' condition, and then an extra full
- OLD -     app loop just for the low condition, e.g.
- OLD - 
- OLD -                  1              0             1               1
- OLD -                _______         ___         _______         _______
- OLD -        TX:    |       |       |   |       |       |       |       |
- OLD -            ___|       |_______|   |_______|       |_______|       |_______
- OLD - 
- OLD -  APP LOOP: ___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
- OLD -    ITERS
- OLD - 
- OLD -     ..so just sending 4 bits can take e.g. 16 app loop iters, more if
- OLD -     the data is all '1's, unless the bit spacing is kept consistent.
- OLD - 
- OLD -     Reading the data probably needs to be interrupt driven to give
- OLD -     accurate bit timings.
- OLD - 
- OLD -     The SECONDARY should always be in READ MODE until it's received
- OLD -     a valid transmission from the PRIMARY, and only then can it reply
- OLD -     with its own data, and the PRIMARY will switch to input mode after
- OLD -     sending the last bit of its data.
- OLD - 
- OLD -     Whenever a CPU switches to 'input mode', the receiver array should
- OLD -     be zeroed, and the interrupt should be enabled for any transition
- OLD -     on that input.
- OLD - 
- OLD -     In single board situations, the PRIMARY simply receives no data
- OLD -     from the remote, the input held high by its WPU (Weak Pull Up),
- OLD -     disabling any kind of ring signaling etc.
- OLD - 
- OLD -     This means there must be NO NOISE AT ALL on the interlink input,
- OLD -     or you'll get phantom ringing for lines #3 and #4!
- OLD - 
- OLD -     The receiver interrupt handler can simply grab snapshots of the hardware
- OLD -     timer (TMR0) into an array that can later be parsed to compile the bits
- OLD -     that were received, and these flags can replace the previous read data
- OLD -     on the next iteration of the event loop, and the xmit/recv process can
- OLD -     simply be restarted:
- OLD - 
- OLD -             PRIMARY                               SECONDARY
- OLD -             -------                               ---------
- OLD -             1. INTS OFF, OUTPUT MODE              1. INTS ON, INPUT MODE
- OLD -             2. SENDS 8 BITS                       2. READS 8 BITS
- OLD -             3. INTS ON, SWITCHES TO INPUT MODE    3. INTS OFF, SWITCH TO OUTPUT MODE
- OLD -             4. READS 8 BITS                       4. SENDS 8 BITS
- OLD -             (repeat)                              (repeat)
- OLD - 
- OLD -     The current application loop is 125x iters/sec, with hardware timer (TMR1)
- OLD -     enforcing the time between iters by doing a spin on the timer's value.
- OLD - 
- OLD - 
- OLD -     -- TBD --
- OLD - 

