// IMPORTANT(Ryan): Remove #defines to enable peripherals when required
//#include "stm32f4xx_hal_conf.h"

// TODO(Ryan): Know general properties of MCU families, e.g. STM32F7s etc.

// TODO(Ryan): Seems that in cubeIDE configuration can specify using LL instead of HAL for BSP generation?

// TODO(Ryan): Have macro definition like in stb libraries to allow for mocking

// IMPORTANT(Ryan): For a function to be mocked/wrapped, it must be in a separate translation unit
// In other words, only function declaration can be present

// IMPORTANT(Ryan): When selecting raw-mcu, will come in different package sizes, i.e number of pins.
// So, a smaller package size might not have pins mapped to a peripheral on the MCU, e.g I2S

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
#include "stm32f429zitx-timer.c"
#include "stm32f429zitx-dio.c"
#include "stm32f429zitx-mem.c"
#include "blinky.c"

// TODO(Ryan): If code size a problem perhaps:
#if 0
#define LIT(x) (sizeof(x) < MAX_LIT_LEN ? (x) : 0)
#endif

// TODO(Ryan): Support 'uart ?'
INTERNAL CONSOLE_CMD_STATUS
console_uart_cmd_system_status_cmd(String8Node *args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  console_printf("uart status command\n");

  result = CONSOLE_CMD_STATUS_SUCCEEDED;

  return result;
}

typedef struct Stat Stat;
struct Stat
{
  u32 accum_ms;
  u32 start_ms;
  u32 min;
  u32 max;
  u32 samples;
  b32 started;
};

INTERNAL Stat
get_stat(void)
{
  Stat result = ZERO_STRUCT;
  result.min = MAX_U32;

  return result;
}

INTERNAL void
stat_update(Stat *stat)
{
  u32 now_ms = HAL_GetTick();

  u32 duration = now_ms - stat->start_ms;
  stat->accum_ms += duration;
  INC_SATURATE_U32(stat->samples);

  if (duration > stat->max)
  {
    stat->max = duration;
  }
  if (duration < stat->min)
  {
    stat->min = duration;
  }

  stat->start_ms = now_ms;
}

// Seems that us is more likely granularity for function execution
INTERNAL u32
stat_avg_us(Stat* stat)
{
  if (stat->samples == 0)
  {
    return 0;
  }
  else
  {
    return (stat->accum_ms * 1000) / stat->samples;
  }
}

void 
EXTI15_10_IRQHandler(void)
{
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13))
  {
    console_printf("interrupt triggered\n");   
    // IMPORTANT(Ryan): Interrupt flags must be manually cleared to avoid retriggering
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
  }
}

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

  // TODO(Ryan): Seem to get framing errors on startup?
  u32 console_cmd_str_buf_len = 64;
  if (stm32f429zitx_create_console(perm_arena, console_cmd_str_buf_len, &uart_params) == STATUS_FAILED)
  {
    while (1) {}
  }
  LOG_DEBUG("Console created\n");

  // TODO(Ryan): Include POST console commands 

  // add commands now
  ConsoleCmdSystem *uart_cmd_system = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, ConsoleCmdSystem);
  uart_cmd_system->name = s8_lit("uart");

  ConsoleCmd *uart_cmd_system_status_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(perm_arena, ConsoleCmd);
  uart_cmd_system_status_cmd->name = s8_lit("status");
  uart_cmd_system_status_cmd->help = s8_lit("Prints status, usage: ");
  uart_cmd_system_status_cmd->func = console_uart_cmd_system_status_cmd;

  SLL_QUEUE_PUSH(uart_cmd_system->first, uart_cmd_system->last, uart_cmd_system_status_cmd);
  SLL_QUEUE_PUSH(global_console.first, global_console.last, uart_cmd_system);

  stm32f429zitx_create_timers(perm_arena, 5); 
 

  // digital IO
  // for electronics, anode is where current flows in, so positive.
  // longer lead for LED
  dio_init(perm_arena, 8);

  GPIO_InitTypeDef init = ZERO_STRUCT;
  init.Pin = GPIO_PIN_2;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 green_led_dio_index = dio_add_output(s8_lit("green_led"), &init, GPIOG, 0);

  init.Pin = GPIO_PIN_3;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_error_dio_id = dio_add_output(s8_lit("red_led"), &init, GPIOG, 0);
  global_error_dio_set = dio_output_set;

  // no protection against hard faults
  // a lot easier to get a serial connection in the field than to attach a debugger
  // with memory module, can inspect/alter memory-mapped registers
  // look in linker map file to get addresses of globals and statics
  // could look at what is at start of ram or flash (comparing u8 and u32 tells endianness)
  // a watchdog timer could reset and get ourselves out of fault handlers 
  mem_add_console_cmds();


  // various ways to implement state machine
  // some use tables for many states
  // for simple ones, use this timer

  // state machine: (typically deterministic?)
  //   * states (Moore produces output when remaining?). Always have fail state (even if implicit)
  //   * transitions (Meley produces output)
  //   * events

  // seems that Moore and Meley definitions are too rigid, as sometimes (produce output/perform action) on a transition and sometimes won't

  // FSMs are useful in times where you have to wait and don't have threads?
  // FSMS good for reactive systems (of which embedded are)

  // event-driven and table-driven FSMs?
  // like autosar/cubeIDE embedded has a lot of 'model-driven' software development
  blinkies_init(perm_arena, 4);
  u32 green_blinky = blinky_create(green_led_dio_index, 10, 500, 1000, 1000);
  //blinky_start(green_blinky);
   
  // IMPORTANT(Ryan): shared interrupts for exti, high-res timers etc.
  // schematic ferrite-bead is inductor? basically suppresion for high frequencies, so suggest susceptible to HF noise 
  // non-maskable NRST will typically have internal pull-up on it
  // IMPORTANT(Ryan): HAL overcomplicates this initialisation
  init.Pin = GPIO_PIN_13;
  init.Mode = GPIO_MODE_IT_RISING;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_LOW;
  u32 user_btn_dio_index = dio_add_output(s8_lit("user_btn_exti"), &init, GPIOC, 0);
  // IMPORTANT(Ryan): Most onboard buttons will have some debouncing circuitry already on them   
  NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 10, 0));
  NVIC_EnableIRQ(EXTI15_10_IRQn);


  // interesting US GPS is free GNSS
  
  // important to know how to write to flash, e.g. log data and when connection to wireless available, transfer
  
  // connected with 2 channel stepper motors, 4 channel DC motors, 8 channel servos,
  // mean stepper motors use more current?


  // GPIO_MODE_EVT_FALLING
  // an event is a software controlled flow control mechanism

  // low power devices seem just to refresh every 30minutes?
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


  // TODO(Ryan): Count and store any runtime update errors under 'main'
  // typical to log error and then update counter
  // TODO(Ryan): Count super loop min, max, avg time (microsecond average loop time is more likely, which SysTick won't see)
  // IMPORTANT(Ryan): Super loop has weaknesses. Implication of say, a GPS module performing trigonometry in 10ms, means
  // other modules have to wait 10ms before they can run.
  // In an RTOS, we could have the GPS prempted.
  // However, an RTOS brings in design issues that must be solved, i.e. accessing data from multiple threads
  // the DWT_CYCCNT register is implementation dependent
  Stat loop_stat = get_stat();

  // IMPORTANT(Ryan): Expect serial terminal to append newline
  while (FOREVER)
  {
    stat_update(&loop_stat);
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
        global_console.cmd_str.str[global_console.cmd_str_buf_cursor++] = (u8)console_ch;
        global_console.cmd_str.size += 1;
        console_ch = console_read_ch();
      }
    }

    // can print higher resolution by computing average
    // printf("Super loop samples=%lu min=%lu ms, max=%lu ms, avg=%lu us\n",
    //        stat_loop_dur.samples, stat_loop_dur.min, stat_loop_dur.max,
    //        stat_dur_avg_us(&stat_loop_dur));

    // this should remain last?
    timers_update();

      // IMPORTANT(Ryan): Ozone won't load symbol if not called directly.
      // So, unfortunately cannot call from a macro to have it easily compiled out
      // __bp();

  }

  return 0;
}


