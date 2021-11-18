// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

                      Digital Signal Debounce and Hysteresis
                      --------------------------------------
                        author: erco@seriss.com 04/28/2019

OVERVIEW
--------

  The 'Debounce' struct has variables to implement digital signal noise rejection
  and "snap-action" hysteresis to avoid oscillations during transitions.

  The structure is implemented as 6 integer values; 2 dynamic that change with live data,
  and 4 constants that are set on initialization, and don't change thereafter:

        typedef struct {
            int value;          // current running value counter (in msecs)
            int max_value;      // (const) 'value' will be clamped to this maximum
            int on_thresh;      // (const) 'value' must count above this to "snap on"
            int off_thresh;     // (const) 'value' must count below this to "snap off"
            int thresh;         // 'current' threshold value implementing 'snap-action' hysteresis
        } Debounce;

  The structure is initialized with values specific to the application this way:

    // Initialize debounce struct
    //
    //     'value' range:
    //         max_value     20         __________________
    //                                 /
    //         on_thresh     15 ....../...................
    //                               /
    //         off_thresh    8 ...../.....................
    //                       0 ____/
    //
    //
    void RotaryDebounceInit(Debounce *d) {
        d->value      = 0;
        d->max_value  = 20;
        d->on_thresh  = 15;
        d->off_thresh = 8;
        d->thresh     = 15;
    }

  ..where the non-zero values would be determined empirically, depending on the timing needs
  of the application.

  The values represent the polling speed; if the polling speed is 1 msec between polls,
  then the values represent msec timing; on_thresh=15 means the state of the input must
  be 'on' for 15 msecs (15 polls of the input being '1').

  All the values in the structure represent how long the digital input signal has been
  in an on or off state, "how long" depenedent on the polling speed.

  Operation Example
  -----------------
  Let's say 'thresh' is set to 'on_thresh', and value is 5.
      If value is 5 and the input polls as 1, value increments to 6.
      If value is 5 and the input polls as 0, value decrements to 4.
  The running 'value' will not go below 0 or above max_value.

  If 'value' climbs past the 'thresh' value, then:
      > 'thresh' is loaded with the value of 'off_thresh'
      > The output snaps to 1, where it will remain until value falls below 'thresh' (off_thresh)

  So once 'thresh' is passed, output snaps to 1, and the thresh value becomes off_thresh,
  lowering the bar for value to have to descend below in order to transition the output
  back to 0.

  The output of the class is based comparing 'value' to 'thresh':

      If value > thresh, output is 'on'
      If value < thresh, output is 'off'

  When the output is 1, thresh has already been set to 'off_thresh', so value
  must go below that value before output can be 0. And when it does, thresh will
  be set to 'on_thresh', ensuring the output remains 0 until value climbs past it again.

  So basically, for a transition in either direction:

      > There has to be enough polls of 1 for 'value' to climb above 'on_thresh'
      > There has to be enough polls of 0 for 'value' to fall below 'off_thresh'

  This technique "cleans up" both sides of the signal; transitions to high, and transitions
  to low.

  The code to implement the algorithm that makes use of the struct looks like this:

      // Manage reading the raw input hardware signal and clean it up,
      // using the Debounce struct to manage the signal processing.
      //
      void ManageInput(Debounce *d) {
          if ( DIGITAL_INPUT ) {          // this is the hardware input bit
              // Hardware bit ON? Might be noise, debounce..
              if ( d->value < d->max_value ) {
                  d->value += G_msecs_per_iter;
                  if ( d->value > d->on_thresh ) {
                      // SNAP ON. No longer "off" until above off_thresh
                      d->thresh = d->off_thresh;
                  }
              }
          } else {
              // Hardware bit OFF? Might be noise, debounce..
              if ( d->value > 0 ) {
                  d->value -= G_msecs_per_iter;
                  if ( d->value < d->off_thresh ) {
                      // SNAP OFF. No longer "on" until above on_thresh
                      d->thresh = d->on_thresh;
                  }
              }
          }
      }

 This implements signal cleanup of the raw hardware digital input signal 'DIGITAL_INPUT',
 the resulting cleaned up digital output accessible by comparing these two values:

     // Read the cleaned up input signal, and act upon it
     if ( d->value > d->thresh ) input_is_on();
     else                        input_is_off();

 See below for an actual walk through of how this algorithm/struct combo works.

EXAMPLE OF SIGNAL CLEANUP
-------------------------

  Consider the following noisy digital input signal, which has excessive noise on both
  the rising and falling edge.

      DIGITAL INPUT SIGNAL:
                             _     ____   _______________       __          _
                            | | |||    | |               | ||| |  || |     | |
                 ___________| |_|||    |_|               |_|||||  ||_|_____| |______


  We want to clean that up, rejecting the noise with an average weighting
  and and hysteresis to snap on or off to prevent quick oscillations,
  so that we get:

      DESIRED OUTPUT SIGNAL:
                                        _______________________________
                                       |                               |
                 ______________________|                               |____________


  To do this, we create a "signal chaser" for the digital input signal; a value that
  might represent either voltage over time, or perhaps better, represents a time tally
  of how long the input digital signal is hi or low.

  We use time, so that the running 'value' represents milliseconds the signal was either
  on or off.

  So in the sample loop for the digital input signal, we count the value up or down by
  the number of millisecs between samples, depending on if the input signal is high or low.
  The result is an integrator of sorts:

      DIGTITAL INPUT SIGNAL:
                         _     ____   _______________       __          _
                        | | |||    | |               | ||| |  || |     | |
             ___________| |_|||    |_|               |_|||||  ||_|_____| |______

                       .<-- Noise -->.               .<----- Noise ----->.

      INTEGRATOR SIGNAL:
                                        _____________        _                      -- max value
                                   ^   /             \      / \/\_
                                  / \ /               \/\/\/      \
                                 /   v                             \     _
                         -     _/                                   \   / \
             ___________/ \_/\/                                      \_/   \____    __ 0


  In our 'Debounce' structure, this integrator is the integer 'value', which is clamped
  to the range 0 to 'max_value'. We then predetermine, typically empirically, the optimal
  on/off threshold values for snapping our output digital signal on and off:

      INPUT SIGNAL:      _     ____   _______________       __          _
                        | | |||    | |               | ||| |  || |     | |
             ___________| |_|||    |_|               |_|||||  ||_|_____| |______
                       .           . .               .                   .
                       .<-- Noise -->.               .<----- Noise ----->.
                       .           . .               .                   .
                       .           . .
                                   .
                                   .
      INTEGRATOR:                  .    _____________        _
                                   ^   /             \      / \/\_
  on_thresh  - - - - - - - - - - -/ \ /- - - - - - - -\/\/\/ - - -\ - - - - - - - on_thresh
                                 / . v                             \     _
  off_thresh - - - - - - - - - _/- . - - - - - - - - - - - - - - - -\ - / \ - - - off_thresh
             ___________/ \_/\/    .                                .\_/   \____
                                   .<-- crosses "on" threshold      .
                                   .                                .<-- crosses "off" threshold
                                   .                                .
                                   .                                .
      OUTPUT SIGNAL:               .________________________________.
                                   |                                |
             ______________________|                                |____________


  To implement hysteresis, the Debounce structure uses the other dynamic
  variable 'thresh' to use as the current 'crossing value'. When the
  signal is low, thresh is set to the 'on threshold', which is what the
  integrator must reach to transition the output to high. Once reached,
  immediately the 'thresh' value is set to the 'off threshold' value,
  so that the integrator must now go BELOW that value to transition
  the output signal back to low.

  So in main(), we create an instance of this struct, and pass it to the above
  functions to initialize and use the structure for live data processing:

      int main() {
          Debounce deb;           // instance of the struct
          Init(&deb);             // initialize the struct
          while(1) {
              ManageInput(&deb);  // use the struct to manage input signal

              // Now take action, using the 'cleaned up' input signal.
              if ( deb.value > deb->thresh ) do_something_when_hi();
              else                           do_something_when_lo();
          }
      }

  So, instead of reading the raw, noisy input signal directly:

      // BAD: Read noisy signal directly
      if ( DIGITAL_INPUT ) do_something_when_high();
      else                 do_something_when_low();

  ..we read the "cleaned up" signal instead this way:

      // GOOD: Read clean signal
      if ( deb.value > deb->thresh ) do_something_when_hi();
      else                           do_something_when_lo();

  To hide the details of the 'if ( deb.value > deb->thresh )', use
  a function like IsXxx(), where Xxx is your signal's name.

  Example: if the hardware input is a "rotary switch" detector, then
  one might use a function named IsRotary() to return the cleaned
  up signal:

      // Return cleaned up state of hardware rotary switch
      int IsRotary(Debounce *d) {
          return( (d->value > d->thresh) ? 1 : 0);
      }

  ..and then to manage and test the value:

      int main() {
          Debounce rot;
          Init(&rot);
          ..
          while (1) {         // main loop

              // Sample the input data and clean it up
              ManageRotaryInput(&rot);

              // Take action on cleaned up signal here
              if ( IsRotary(rot) ) do_something_when_rotary_on();
              else                 do_something_when_rotary_off();
          }
      }

  ..where 'IsRotary()' makes for more readable, "action verb" oriented code.
