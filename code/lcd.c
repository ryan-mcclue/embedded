// SPDX-License-Identifier: zlib-acknowledgement

// https://www.youtube.com/watch?v=TUD4qnQjSzs&list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&index=28&t=713s

// reflective layers cost more, but can be lower power as less backlight required (however only effective in well-lit areas)

// dot matrix monochrome, e.g. traffic signals

// lower-power than TFT
// less memory due to CGROM/CGRAM
// cheaper

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
// TODO(Ryan): Investigate code space saved specifying addresses instead of including cmsis
#define INIT_CYCLE_COUNTER() \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk

#define ENABLE_CYCLE_COUNTER() \
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk

#define DISABLE_CYCLE_COUNTER() \
    DWT->CTRL &=  ~DWT_CTRL_CYCCNTENA_Msk

#define GET_CYCLE_COUNTER() \
    DWT->CYCCNT

#define RESET_CYCLE_COUNTER() \
    DWT->CYCCNT = 0

INTERNAL void 
delay_us(u32 us)
{
  u32 start_cycles = GET_CYCLE_COUNTER();
  u32 cycles_per_us = (HAL_RCC_GetHCLKFreq() / 1000000);
  u32 cycles_to_delay = us * cycle_per_us;
  while ((GET_CYCLE_COUNTER() - start_cycles) < cycles_to_delay);
}


// TODO(Ryan): Segger SystemView (https://mcuoneclipse.com/2015/11/16/segger-systemview-realtime-analysis-and-visualization-for-freertos/)

// TODO(Ryan): Understand Systick/RCC more in depth (weeW systick microsecond; matej;)
// TODO(Ryan): Verify clock signal by measuring GPIO pin toggling frequency

INTERNAL void
lcd_write(u8 data)
{
  dio_output_set(rs_dio, 1);
  dio_output_set(rw_dio, 0);
  delay_us(10);
  dio_output_set(e_dio, 1);
  delay_us(5);
  // IMPORTANT(Ryan): This is where pin ordering helps  
  GPIOE->ODR &= 0xFF00;
  GPIOE->ODR |= data;
  delay_us(10);
  dio_output_set(e_dio, 0);
}

INTERNAL void
lcd_cmd(u8 data)
{
  dio_output_set(rs_dio, 0);
  dio_output_set(rw_dio, 0);
  delay_us(10);
  dio_output_set(e_dio, 1);
  delay_us(5);
  GPIOE->ODR &= 0xFF00;
  GPIOE->ODR |= data;
  delay_us(10);
  dio_output_set(e_dio, 0);
}

INTERNAL void
init_lcd1602(void)
{
  // init pins  
  // ...
  
  HAL_Delay(20);
  // 8bit, 2lines, 5x11 font size
  lcd_cmd(0x3C);
  HAL_Delay(5);
  // on
  lcd_cmd(0x0C); 
  HAL_Delay(5);
  // clear display
  lcd_cmd(0x01); 
  HAL_Delay(5);
  // put cursor at start
  lcd_cmd(0x02); 
  HAL_Delay(5);
}
