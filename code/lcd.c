// SPDX-License-Identifier: zlib-acknowledgement

// https://www.youtube.com/watch?v=TUD4qnQjSzs&list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&index=28&t=713s

// reflective layers cost more

// dot matrix monochrome, e.g. traffic signals

// seems that would use 16 pins character-LCD as is IPS?
// perhaps lower-power and cheaper than SPI?

// Vdd is voltage for logic operation
// V0 is adjustable voltage for LCD display (can be GND for stronger lighting?)

// PD0-7 D0-7
// PD11 - RS
// PD12 - R/W 
// PD13 - E 

// lcd 1602

// DWT (debug watchpoint and trace)
// HAL_Delay() not accurate, e.g. if 0.8 ticks when called, only 0.2ticks elapsed

// NOTE(Ryan): Already enabled if connected via debugger
#define INIT_CYCLE_COUNTER() \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#define ENABLE_CYCLE_COUNTER() \
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk

#define DISABLE_CYCLE_COUNTER() \
    DWT->CTRL &=  ~DWT_CTRL_CYCCNTENA_Msk

#define GET_CYCLE_COUNTER() \
    DWT->CYCCNT

#define RESET_CYCLE_COUNTER() \
    DWT->CYCCNT = 0

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t au32_microseconds)
{
  uint32_t au32_initial_ticks = DWT->CYCCNT;
  uint32_t au32_ticks = (HAL_RCC_GetHCLKFreq() / 1000000);
  au32_microseconds *= au32_ticks;
  while ((DWT->CYCCNT - au32_initial_ticks) < au32_microseconds-au32_ticks);
}

__STATIC_INLINE void DWT_Delay_us(volatile uint32_t au32_microseconds)
{
  u32 start_cycles = GET_CYCLE_COUNTER();
  u32 us_tick_freq = (HAL_RCC_GetHCLKFreq() / 1000000);
  u32 us_ticks *= us_tick_freq;
  while ((DWT->CYCCNT - start_cycles) < us_ticks - us_tick_freq);
}

__STATIC_INLINE void DWT_Delay_ms(volatile uint32_t au32_milliseconds)
{
  uint32_t au32_initial_ticks = DWT->CYCCNT;
  uint32_t au32_ticks = (HAL_RCC_GetHCLKFreq() / 1000);
  au32_milliseconds *= au32_ticks;
  while ((DWT->CYCCNT - au32_initial_ticks) < au32_milliseconds);
}


// TODO(Ryan): Segger SystemView (https://mcuoneclipse.com/2015/11/16/segger-systemview-realtime-analysis-and-visualization-for-freertos/)

// TODO(Ryan): Understand Systick/RCC more in depth (weeW systick microsecond; matej;)
// TODO(Ryan): Verify clock signal by measuring GPIO pin toggling frequency

GPIOE
INTERNAL void
init_lcd1602(void)
{
  // init pins  

  // data register
  dio_set_output(rs_dio, 1);
  dio_set_output(rw_dio, 0);
}
