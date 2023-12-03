// SPDX-License-Identifier: zlib-acknowledgement

// logic analyser to measure time between toggles

// poor mans profiling, i.e. sampler profiling counting cycles 
// (would require code instrumentation to see what blocking on)

// parsing a collection of debugger backtracers could be used to ascertain main time spent in
// (however, introducing say GDB has huge overhead)

// ITM cortex-m4 one-way asynchronous transmission through SWO pin via dedicated bus
// much faster than UART, and larger frame size means less register writes
// SWO pin may be connected to USB port already if ICDI present on board
// STM32F429-Discovery: worked without any problem, SB9 solder bridge required before to be soldered
// (SWO might require solder bridge enabling)
// ITM sends frames
// ITM typically used as a printf for logging

// ITM .jdebug script files?

// so, if using SWO printf would just be plain ASCII?
// ITM implies framing
// however, both use the ITM hardware as SWO is a trace pin

// void putc(char c) {
//   while (ITM_STIM32(0) == 0);
//   ITM_STIM8(0) = c;
// }

// ARM Debug and Trace:
//  - ITM
//  - SWD: cortex-M
//     - SWDIO
//     - SWCLK
//     - optional SWO pin (can output ITM info)

// DWT cortex-m4 (cycles only counted when CPU is running)
// (measuring time between cycles, must take into account cycles reading actual cycle counter variable)

// semihosting is slow?

// SWO and ITM packet format automatically work with Ozone?

// TRACE pins different used with J-Trace?
// TRACE is multiple pins so faster than single SWO pin of SWD header?

