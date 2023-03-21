// SPDX-License-Identifier: zlib-acknowledgement


// IMPORTANT(Ryan): Distinction between hardware/board version and software version
// IMPORTANT(Ryan): Pins can be shuffled around during development, 
// e.g. EE finds routing a certain way easier
// So, put pins here:
#if BOARD_VERSION_X
  #define PIN_SOMETHING 1
#else
  #define PIN_SOMETHING 2
#endif

// Would be nice if could read board version from hardware,
// e.g. 4 unused GPIO pins. Reading them tells you what board version, e.g. know when say updated an accelerometer sensor

// CubeMX perhaps useful for detecting pin conflicts 
