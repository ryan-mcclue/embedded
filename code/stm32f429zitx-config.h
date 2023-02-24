// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(CONFIG_H)
#define CONFIG_H

// TODO(Ryan): Add macro conditionals for features
// e.g. #if defined(CONFIG_FEATURE_TEMPERATURE_SENSING)

typedef u32 STATUS;
enum
{
  STATUS_FAILED = 0,
  STATUS_SUCCEEDED,
};

// TODO(Ryan): Investigate cortex-m architecture like interrupt registers like PRIMASK

// NOTE(Ryan): No nested interrupts at all
#define ATOMIC_BEGIN() __disable_irq()
#define ATOMIC_END() __enable_irq()

// NOTE(Ryan): Using an exception mask register, allow nested interrupts in the form of NMI and HardFault
GLOBAL volatile u32 global_primask_reg;
#define CRITICAL_BEGIN() \
    do { \
        global_primask_reg = __get_PRIMASK(); \
        __set_PRIMASK(1); \
    } while (0)
#define CRITICAL_END() \
    do { \
        __set_PRIMASK(global_primask_reg); \
    } while (0)


#endif
