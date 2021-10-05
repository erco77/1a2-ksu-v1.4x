#ifndef TIMERMSECS_H
#define TIMERMSECS_H

//
// TimerMsecs -- A generic count-up auto-resetting timer
//
//     VERS DATE       AUTHOR
//     1.00 09/17/2019 erco@seriss.com
//

//	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//      Copyright (C) 2019 Seriss Corporation.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Struct to manage each counter
typedef struct {
    char run;       // 1:counter running, 0: counter stopped (won't Advance if run=0)
    int  max;       // value for timer to run out when reached
    int  msecs;     // running timer (in msecs)
} TimerMsecs;

// Initialize timer to being stopped and zeroed
inline void Init_TimerMsecs(TimerMsecs *t) {
    t->max   = 0;
    t->msecs = 0;
    t->run   = 0;
}

// Start timer, set count
inline void Set_TimerMsecs(TimerMsecs *t, int val) {
    t->max   = val; // set max
    t->msecs = 0;   // zero counter
    t->run   = 1;   // start running
}

// Stop timer, set count
inline void Stop_TimerMsecs(TimerMsecs *t) {
    t->run   = 0;   // stop
    t->msecs = 0;
}

// See if timer is stopped
inline int IsStopped_TimerMsecs(TimerMsecs *t) {
    return (t->run == 0) ? 1 : 0;
}

// See if timer is running
inline int IsRunning_TimerMsecs(TimerMsecs *t) {
    return t->run;
}

// Return current timer value in msecs
inline int Get_TimerMsecs(TimerMsecs *t) {
    return t->msecs;
}

// Advance the specified msecs timer by G_msecs_per_iter, and check if max reached
// Returns:
//     1 - max reached, auto-reset to zero
//     0 - max NOT reached (or timer stopped)
//
inline int Advance_TimerMsecs(TimerMsecs *t,            // timer to adjust
                              int msecs) {              // #msecs to advance counter
    if ( ! t->run ) return 0;
    if ( (t->msecs += msecs) < t->max ) {
        // not at max yet, return 0
        return 0;
    } else {
        // Hit max? Zero timer and return 1
        t->msecs = 0;                                   // also reset if not oneshot
        return 1;
    }
}

#endif
