// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(CONFIG_H)
#define CONFIG_H

// TODO(Ryan): Add macro conditionals for features
// e.g. #if defined(CONFIG_FEATURE_TEMPERATURE_SENSING)

typedef u32 STATUS;
enum
{
  STATUS_SUCEEDED,
  STATUS_FAILED,
};

// IMPORTANT(Ryan): File naming convention
// #define CONFIG_CONSOLE_UART USART3

#endif
