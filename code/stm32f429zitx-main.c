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
#include "stm32f429zitx-console.c"

// TODO(Ryan): Support 'uart ?'
INTERNAL CONSOLE_CMD_STATUS
console_uart_cmd_system_status_cmd(String8Node *args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  console_printf("uart status command\n");

  result = CONSOLE_CMD_STATUS_SUCCEEDED;

  return result;
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
    while (1) {}
  }

  MemArena *perm_arena = mem_arena_allocate(KB(32));

  initialise_global_temp_mem_arenas(KB(32));
  TempMemArena temp_arena = temp_mem_arena_get(NULL, 0);

  UartParams uart_params = ZERO_STRUCT;
  uart_params.tx_pin = GPIO_PIN_8;
  uart_params.rx_pin = GPIO_PIN_9;
  uart_params.af = GPIO_AF7_USART3;
  uart_params.gpio_base = GPIOD;
  uart_params.baud_rate = 57600;
  uart_params.uart_base = USART3;
  uart_params.rx_buf_len = 256;
  uart_params.tx_buf_len = 256;

  u32 console_cmd_str_buf_len = 64;
  if (stm32f429zitx_create_console(perm_arena, console_cmd_str_buf_len, &uart_params) == STATUS_FAILED)
  {
    while (1) {}
  }

  LOG_INFO(main, "created console\n");

  // add commands now
  ConsoleCmdSystem *uart_cmd_system = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, ConsoleCmdSystem);
  uart_cmd_system->name = s8_lit("uart");

  ConsoleCmd *uart_cmd_system_status_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, ConsoleCmd);
  uart_cmd_system_status_cmd->name = s8_lit("status");
  uart_cmd_system_status_cmd->help = s8_lit("Prints status");
  uart_cmd_system_status_cmd->func = console_uart_cmd_system_status_cmd;

  SLL_QUEUE_PUSH(uart_cmd_system->first, uart_cmd_system->last, uart_cmd_system_status_cmd);
  SLL_QUEUE_PUSH(global_console.first, global_console.last, uart_cmd_system);


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
    // IMPORTANT(Ryan): First exposure to synchronisation issues
    // UART is slow, compared to CPU frequency
    // Therefore, can't just willy nilly print over with a small ring buffer size
    // if we were to just loop over until '\n', would get way too many buffer overruns as running way to quick

    char console_ch = console_read_ch();
    while (console_ch != 0 && global_console.cmd_str_buf_cursor != global_console.cmd_str_buf_len)
    {
      if (console_ch == '\n')
      {
        console_execute_cmd(global_console.cmd_str);
        global_console.cmd_str.size = 0;
        global_console.cmd_str_buf_cursor = 0;
        break;
      }
      else
      {
        global_console.cmd_str.str[global_console.cmd_str_buf_cursor++] = console_ch;
        global_console.cmd_str.size += 1;
        console_ch = console_read_ch();
      }
    }
      // IMPORTANT(Ryan): Ozone won't load symbol if not called directly.
      // So, unfortunately cannot call from a macro to have it easily compiled out
      // __bp();
  }

  return 0;
}


