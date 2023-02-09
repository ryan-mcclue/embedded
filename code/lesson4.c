// SPDX-License-Identifier: zlib-acknowledgement
/*
 * code: led triggered by button with interrupt and debouncing (with timer or hardware debouncer?)
 * want to debounce say no more than 10Hz to achieve an approx. 16ms user response?
 * have led blink at precise rate
 * understand systick
 *
 * interrupts can be disabled (at this time all further ISRs are pending and resolved based on priority) when in ISR and reenabled on exit.
 * TODO: What determines interrupt priority?
 * 
 * displays are commonly used in embedded systems that are not battery operated as consume a lot of power
 *
 * qfsm tool for creating state machines?
 *
 */
