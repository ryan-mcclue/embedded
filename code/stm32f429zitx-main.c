// IMPORTANT(Ryan): Remove #defines to enable peripherals when required
//#include "stm32f4xx_hal_conf.h"

// TODO(Ryan): Have macro definition like in stb libraries to allow for mocking

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

GLOBAL UART_HandleTypeDef *global_console_uart_handle;

// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

#if 0
typedef u32 (*console_cmd_func)(String8List *args);

typedef struct ConsoleCmd ConsoleCmd;
struct ConsoleCmd
{
  ConsoleCmd *next;
  String8 name, description, help;

  // all commands should print text like OK 
  // and return say 1 to allow for testing
  console_cmd_func func;
};

typedef struct ConsoleCmdSystem ConsoleCmdSystem;
struct ConsoleCmdSystem
{
  ConsoleCmdSystem *next;
  String8 name, description;

  // local log level variable

  ConsoleCmd *first;
  ConsoleCmd *last;
};

// This will be a global variable?
typedef struct Console Console;
struct Console
{
  // global log level variable

  ConsoleCmdSystem *first;
  ConsoleCmdSystem *last;

  // contain uart handle
};

INTERNAL u32
uart_status(String8List *cmds)
{
  u32 result = 0;

  TempMemArena temp_mem_arena = temp_mem_arena_get(NULL, 0);

  if (cmds->node_count == 2)
  {
    String8Node *name_arg = cmds->first;
    ASSERT(s8_match(name_arg.string, s8_lit("uart"), S8_MATCH_FLAG_CASE_INSENSITIVE));

    String8Node *status_arg = name_arg->next;
    if (s8_match(status_arg->string, s8_lit("status"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      String8 message = s8_lit("Status is a-okay!\n");
      HAL_StatusTypeDef hal_status = HAL_UART_Transmit(global_console_uart_handle, message.str, (u16)message.size, 500);
    }
    else
    {
      result = 1;
    }
  }
  else
  {
    result = 1;
  }

  temp_mem_arena_release(temp_mem_arena);

  return result;
}

// ?
// log, log off
// TODO: * log info (would act on all subsystems)

// uart ?, uart help
// uart status (show clients and their rx,tx index and buffer information)
// uart log (get log level), uart log off, uart log info
// uart stats, uart stats clear
// uart test

INTERNAL void
execute_console_cmd(MemArena *arena, ConsoleState *console_state, String8 cmd)
{
  String8 space_split = s8_lit(" ");
  String8List cmd_tokens = s8_split(arena, cmd, &space_split, 1);

  String8Node *system_token = cmd_tokens.first;
  String8Node *name_token = system_token->next;

  for (ConsoleCmd *console_cmd = console_state->first;
       console_cmd != NULL;
       console_cmd = console_cmd->next)
  {
    if (s8_match(system_token->string, console_cmd->system, S8_MATCH_FLAG_CASE_INSENSITIVE) &&
        s8_match(name_token->string, console_cmd->name, S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      if (console_cmd->func(&cmd_tokens) == 1)
      {
        HAL_StatusTypeDef hal_status = HAL_UART_Transmit(global_console_uart_handle, console_cmd->help.str, (u16)console_cmd->help.size, 500);
      }

      break;
    }
  }
}
#endif

#if defined(TEST_BUILD)
int testable_main(void)
#else
int main(void)
#endif
{
  // ./configure --help 
  // ./configure --target-list="arm-softmmu"
  // make -j$(getconf _NPROCESSORS_ONLN)

  // TODO(Ryan): Add error handling
  stm32f429zitx_initialise();

  // setvbuf(stdout, NULL, _IONBF, 0);
  // TODO(Ryan): In RAM code just calls custom printc(), so why this no buffering call? For streams?

  MemArena *permanent_arena = mem_arena_allocate(KB(32));
  initialise_global_temp_mem_arenas(KB(32));

  TempMemArena temp_arena = temp_mem_arena_get(NULL, 0);

  UartParams uart_params = ZERO_STRUCT;
  uart_params.tx_pin = GPIO_PIN_8;
  uart_params.rx_pin = GPIO_PIN_9;
  uart_params.af = GPIO_AF7_USART3;
  uart_params.gpio_base = GPIOD;
  uart_params.baud_rate = 57600;
  uart_params.uart_base = USART3;

  UartResult uart_result = stm32f429zitx_initialise_uart(&uart_params);
  if (uart_result.status == STATUS_FAILED)
  {
    while (1) {}
  }
  global_console_uart_handle = &uart_result.handle;

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
        char ch = console_buf[i];
        if (ch == '\n') 
        {
          String8 cmd = s8_prefix(console_str, i);
          //execute_console_cmd(temp_arena.arena, &console_state, cmd);
        };
        // hal_status = HAL_UART_Transmit(&uart_result.handle, console_buf + i, 1, 500);
      }
    }

      // IMPORTANT(Ryan): Ozone won't load symbol if not called directly.
      // So, unfortunately cannot call from a macro to have it easily compiled out
      // __bp();

    temp_mem_arena_release(temp_arena);
  }

  return 0;
}


// The log toggle char at the console is ctrl-L which is form feed, or 0x0c.
#define LOG_TOGGLE_CHAR '\x0c'

typedef u32 LOG_LEVEL;
enum {
  LOG_LEVEL_OFF = 0,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_VERBOSE,
  LOG_LEVEL_COUNT
};

#define LOG_LEVEL_NAMES "off, error, warning, info, debug, verbose"

INTERNAL char *
log_level_str(LOG_LEVEL log_level)
{
  char *result = "unknown";

  switch (log_level)
  {
    default: break;
    case LOG_LEVEL_OFF:
    {
      result = "off"; 
    } break;
    case LOG_LEVEL_ERROR:
    {
      result = "error"; 
    } break;
    case LOG_LEVEL_WARNING:
    {
      result = "warning"; 
    } break;
    case LOG_LEVEL_INFO:
    {
      result = "info"; 
    } break;
    case LOG_LEVEL_DEBUG:
    {
      result = "debug"; 
    } break;
    case LOG_LEVEL_VERBOSE:
    {
      result = "verbose"; 
    } break;
  }

  return result;
}

INTERNAL void 
console_printf(char *fmt, ...);
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
console_printf_nested(char *fmt, va_list args);
{
  TempMemArena arena = temp_mem_arena_get(NULL, 0);
  
  String8 message = s8_fmt_nested(arena.arena, fmt, args);
  
  HAL_StatusTypeDef hal_status = HAL_UART_Transmit(global_console.uart_handle, 
                                                   message.str, (u16)message.size, 500);
  // TODO(Ryan): Add max buffer limit and indicate if reached by appending [!]

  temp_mem_arena_release(arena);
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
    if (global_console.log_active && global_SUBSYSTEM_cmd_system.log_level >= LOG_LEVEL_ERROR) \
      console_log(PASTE("ERROR ", SUBSYSTEM) fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_WARNING(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_SUBSYSTEM_cmd_system.log_level >= LOG_LEVEL_WARNING) \
      console_log(PASTE("WARNING ", SUBSYSTEM) fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_INFO(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_SUBSYSTEM_cmd_system.log_level >= LOG_LEVEL_INFO) \
      console_log(PASTE("INFO ", SUBSYSTEM) fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_DEBUG(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_SUBSYSTEM_cmd_system.log_level >= LOG_LEVEL_DEBUG) \
      console_log(PASTE("DEBUG ", SUBSYSTEM) fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_VERBOSE(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_SUBSYSTEM_cmd_system.log_level >= LOG_LEVEL_VERBOSE) \
      console_log(PASTE("VERBOSE ", SUBSYSTEM) fmt, ##__VA_ARGS__); \
  } while (0) 
