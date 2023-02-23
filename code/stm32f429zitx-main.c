// IMPORTANT(Ryan): Remove #defines to enable peripherals when required
//#include "stm32f4xx_hal_conf.h"

// TODO(Ryan): Have macro definition like in stb libraries to allow for mocking

// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

#include "base-inc.h"

#if defined(TEST_BUILD)
  // IMPORTANT(Ryan): Put these in every file
  // Strange circular includes in st drivers otherwise
  #include "stm32f4xx.h"
  #include "stm32f4xx_hal.h"
#elif defined(SIMULATOR_BUILD)
  #include "stm32f4xx.h"
  #include "stm32f4xx_hal.h"
  #include "syscalls.c" 
  #include "sysmem.c" 
#else
  #include "stm32f4xx_hal_msp.c" 
  #include "stm32f4xx_it.c" 
  #include "syscalls.c" 
  #include "sysmem.c" 
  #include "system_stm32f4xx.c"

  #include "stm32f4xx_hal.c"
  #include "stm32f4xx_hal_cortex.c"
  #include "stm32f4xx_hal_rcc.c"
  #include "stm32f4xx_hal_gpio.c"
  #include "stm32f4xx_hal_uart.c"
#endif

#include "stm32f429zitx-config.h"
#include "stm32f429zitx-boot.h"
#include "stm32f429zitx-uart.c"

typedef u32 LOG_LEVEL;
enum {
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_VERBOSE,
  LOG_LEVEL_COUNT
};

#define LOG_LEVEL_NAMES "error, warning, info, debug, verbose"

INTERNAL char *
log_level_str(LOG_LEVEL log_level)
{
  char *result = "unknown";

  switch (log_level)
  {
    default: break;
    case LOG_LEVEL_ERROR:
    {
      result = "ERROR"; 
    } break;
    case LOG_LEVEL_WARNING:
    {
      result = "WARNING"; 
    } break;
    case LOG_LEVEL_INFO:
    {
      result = "INFO"; 
    } break;
    case LOG_LEVEL_DEBUG:
    {
      result = "DEBUG"; 
    } break;
    case LOG_LEVEL_VERBOSE:
    {
      result = "VERBOSE"; 
    } break;
  }

  return result;
}

typedef u32 CONSOLE_CMD_STATUS;
enum {
  CONSOLE_CMD_STATUS_FAILED,
  CONSOLE_CMD_STATUS_SUCEEDED,
};

typedef CONSOLE_CMD_STATUS (*console_cmd_func)(String8List *args);

typedef struct ConsoleCmd ConsoleCmd;
struct ConsoleCmd
{
  ConsoleCmd *next;
  String8 name, description, help;

  console_cmd_func func;
};

typedef struct ConsoleCmdSystem ConsoleCmdSystem;
struct ConsoleCmdSystem
{
  ConsoleCmdSystem *next;
  String8 name, description;

  ConsoleCmd *first;
  ConsoleCmd *last;
};

typedef struct Console Console;
struct Console
{
  b32 log_active;
  LOG_LEVEL log_level;

  ConsoleCmdSystem *first;
  ConsoleCmdSystem *last;

  MemArena *arena;

  UART_HandleTypeDef *uart_handle;
};

GLOBAL Console global_console;


INTERNAL void 
console_printf(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  TempMemArena arena = temp_mem_arena_get(NULL, 0);
  
  String8 message = s8_fmt_nested(arena.arena, fmt, args);
  
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(global_console.uart_handle, 
                                                   message.str, (u16)message.size, 500);
  // TODO(Ryan): Add max buffer limit and indicate if reached by appending [!]

  temp_mem_arena_release(arena);

  va_end(args);
}

INTERNAL void 
console_printf_nested(char *fmt, va_list args)
{
  TempMemArena arena = temp_mem_arena_get(NULL, 0);
  
  String8 message = s8_fmt_nested(arena.arena, fmt, args);
  
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(global_console.uart_handle, 
                                                   message.str, (u16)message.size, 500);
  // TODO(Ryan): Add max buffer limit and indicate if reached by appending [!]

}

INTERNAL void
console_log(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  u32 ms = HAL_GetTick();

  console_printf("%lu.%03lu ", ms / 1000U, ms % 1000U);
  console_printf_nested(fmt, args);

  va_end(args);
}

#define LOG_ERROR(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_ERROR) \
      console_log("ERROR (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_WARNING(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_WARNING) \
      console_log("WARNING (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_INFO(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_INFO) \
      console_log("INFO (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_DEBUG(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_DEBUG) \
      console_log("DEBUG (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_VERBOSE(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_VERBOSE) \
      console_log("VERBOSE (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 


// uart ?, uart help
// uart status (show clients and their rx,tx index and buffer information)
// uart log (get log level), uart log off, uart log info
// uart stats, uart stats clear
// uart test

INTERNAL void
global_console_execute_cmd(String8 raw_str)
{
  String8 space_split = s8_lit(" ");
  String8List cmd_tokens = s8_split(global_console.arena, raw_str, &space_split, 1);

  String8Node *first_token = cmd_tokens.first;
  // NOTE(Ryan): Handle overarching commands
  if (cmd_tokens.node_count == 1)
  {
    if (first_token->string.str[0] == '?')
    {
      console_printf("Help\n");
    }
    else if (first_token->string.str[0] == '+')
    {
      global_console.log_active = true;
      console_printf("Logging enabled\n");
    }
    else if (first_token->string.str[0] == '-')
    {
      global_console.log_active = false;
      console_printf("Logging disabled\n");
    }
    else if (first_token->string.str[0] == '*')
    {
      char *log_level = log_level_str(global_console.log_level); 
      console_printf("%s logging is %s\n", log_level, global_console.log_active ? "enabled" : "disabled");
    } 
    else if (s8_match(first_token->string, s8_lit("error"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_ERROR; 
      console_printf("Log level set to ERROR\n");
    }
    else if (s8_match(first_token->string, s8_lit("warning"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_WARNING; 
      console_printf("Log level set to WARNING\n");
    }
    else if (s8_match(first_token->string, s8_lit("info"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_INFO; 
      console_printf("Log level set to INFO\n");
    }
    else if (s8_match(first_token->string, s8_lit("debug"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_DEBUG; 
      console_printf("Log level set to DEBUG\n");
    }
    else if (s8_match(first_token->string, s8_lit("verbose"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_VERBOSE;
      console_printf("Log level set to VERBOSE\n");
    }
    else
    {
      console_printf("Unknown command: %.*s\n", s8_varg(first_token->string));
    }
  }
  else
  {
    String8Node *second_token = first_token->next;

    for (ConsoleCmdSystem *console_cmd_system = global_console.first;
         console_cmd_system != NULL;
         console_cmd_system = console_cmd_system->next)
    {
      if (s8_match(first_token->string, console_cmd_system->name, S8_MATCH_FLAG_CASE_INSENSITIVE))
      {
        for (ConsoleCmd *console_cmd = console_cmd_system->first;
             console_cmd != NULL;
             console_cmd = console_cmd->next)
        {
          if (s8_match(second_token->string, console_cmd->name, S8_MATCH_FLAG_CASE_INSENSITIVE))
          {
            if (console_cmd->console_func(tokens) == CONSOLE_CMD_STATUS_FAILED)
            {
              console_printf("%s\n", console_cmd->help);
            }
          }
        }
      }

    }

  }

}

GLOBAL ConsoleCmdSystem global_main_console_cmd_system;

#if defined(TEST_BUILD)
int testable_main(void)
#else
int main(void)
#endif
{
  if (stm32f429zitx_initialise() != STATUS_SUCCEEDED)
  {
    __disable_irq();
    while (1) {}
  }

  MemArena *permanent_arena = mem_arena_allocate(KB(32));
  initialise_global_temp_mem_arenas(KB(32));

  TempMemArena temp_arena = temp_mem_arena_get(NULL, 0);

  // NOTE(Ryan): Initialise global console
  UartParams uart_params = ZERO_STRUCT;
  uart_params.tx_pin = GPIO_PIN_8;
  uart_params.rx_pin = GPIO_PIN_9;
  uart_params.af = GPIO_AF7_USART3;
  uart_params.gpio_base = GPIOD;
  uart_params.baud_rate = 9600;
  uart_params.uart_base = USART3;

  UartResult uart_result = stm32f429zitx_initialise_uart(&uart_params);
  if (uart_result.status == STATUS_FAILED)
  {
    while (1) {}
  }
  global_console.uart_handle = &uart_result.handle;

  global_console.arena = temp_arena.arena;
  global_console.log_active = true;
  global_console.log_level = LOG_LEVEL_VERBOSE;

  LOG_INFO(main, "initialised console");

#if 0
  ConsoleCmd status = ZERO_STRUCT;
  status.system = s8_lit("uart");
  status.name = s8_lit("status");
  status.help = s8_lit("Print status. Usage: uart status\n");
  status.func = uart_status;

  ConsoleState console_state = ZERO_STRUCT;
  SLL_QUEUE_PUSH(console_state.first, console_state.last, &status);
#endif

  // systick is 1ms; not spectacular resolution
  // important to recognise possible rollover when doing elapsed time calculations

  // GPIO_MODE_EVT_FALLING
  // an event is a software controlled flow control mechanism

  // 3 low power modes (usage, startup time, activity)
  // (REQUIRE KNOWLEDGE OF CLOCK SOURCES)
  // 1. sleep: CPUCLK shuts off
  //           have wakeup events (could be a button press) that are distinct from interrupts
  //           HAL_SuspendTick() prior to entering sleep mode
  //           HAL_PWR_EnterSLEEPMode();
  //           HAL_ResumeTick();
  //           However, seems more flexible to use an interrupt (WFI) to wakeup, e.g. timer, SPI bus data etc.
  // 2. stop: most CLKS off, i.e. all peripheral clocks off (if using WFI, can only use EXTI)
  //          no need to explicitly suspend systick
  //          however, on resumption will have to system_intialise(), i.e. start clocks again
  // 3. standby: SRAM contents lost, most CLKS/oscillators off 
  //             will restart system on wakeup
  //             WFI must be on certain pin, e.g. WKUP pin
  //             still have power say on 3.3V pin
  //             can check if wakeup by inspecting backup registers
  //             TODO(Ryan): could also wakeup from watchdog or rtc
  
  // IMPORTANT(Ryan): When enabling interrupt, set its priority before
  
  // TODO(Ryan): UI --> touch controller and LCD

  // TODO(Ryan): matej DMA. have both standard and circular DMA?
  // https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx
  
  // startup test could be checking if writing to external SDRAM works


  // flash into sectors which are composed of pages.
  // only erased in sectors at a time (so much faster than per byte like EEPROM)
  // however, can be written to pages, or even finer
  //
  // flash writing
  // limit flash writes, must erase before writing, must be in sector/page sizes? default erase byte is?
  // When you restart, you scan the memory to find the highest sequence number of a block with a valid CRC
  // In reality most of the blocks in most of the devices can easily reach 3x the limit. With CRC checks you can monitor the integrity of your blocks

  // LED debugging: (create certain patterns to indicate certain errors)
  // green heartbeat led timer indicates if running and if CPU overloaded
  // red led indicates error, say for initialisation error
  // red and green on if in fault handler
  // blink at certain rate for when writing/reading to flash
  // (could solder a current limiting resistor to an LED so can just 'plug-in-play')
  
  // Serial debugging logging
  
  // perhaps implement mocks for simulator also, so can just run anywhere?


  // rtc for times like every 24hours?
  // a timer is just a basic counter and compare
  // rtc more options and on separate power domain, e.g. drift register, crystal source?
  // more accurate readings as direct clock source not PLL?
  // XTAL are pins for external oscillator (on boards, X labels oscillator)
  // RTC stores date, like day of week etc.
  // for accurate, i.e. mininmal drifting stm32 rtc, want it to be clocked with LSE (as its at 32.768KHz which is a sweet spot?) 
  // the LSI is not as accurate? meaning crystal frequency varies?
  // why want low frequency as compared to say HSE?
  // IMPORTANT: LSE must be enabled via RCC
  // RTC operates in backup domain, so on reset will retain (only if power still to VBAT, not necessarily whole system VCC?)
  // Will require initial setting to actual time
  //
  // Not all discovery boards have this on them, rather only have pads for you to solder your own cystal to
  // RTC can be used to timestamp data

  // IMPORTANT(Ryan): Expect serial terminal to append newline
  while (FOREVER)
  {
    u32 console_buf_size = 64;
    u8 *console_buf = MEM_ARENA_PUSH_ARRAY_ZERO(temp_arena.arena, u8, console_buf_size);

    String8 console_str = ZERO_STRUCT;
    console_str.str = console_buf;
    console_str.size = console_buf_size;

    HAL_StatusTypeDef hal_status = HAL_UART_Receive(&uart_result.handle, (u8 *)console_buf, (u16)console_buf_size, 1000);
    if (hal_status != HAL_ERROR)
    {
      for (size_t i = 0; i < console_buf_size; i += 1)
      {
        if (console_buf[i] == '\n') 
        {
          String8 cmd = s8_prefix(console_str, i);
          global_console_execute_cmd(cmd);
        }
      }
    }

      // IMPORTANT(Ryan): Ozone won't load symbol if not called directly.
      // So, unfortunately cannot call from a macro to have it easily compiled out
      // __bp();

    temp_mem_arena_release(temp_arena);
  }

  return 0;
}


