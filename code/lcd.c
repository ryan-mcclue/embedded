// SPDX-License-Identifier: zlib-acknowledgement

// https://www.youtube.com/watch?v=TUD4qnQjSzs&list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&index=28&t=713s

// reflective layers cost more, but can be lower power as less backlight required (however only effective in well-lit areas)

// dot matrix monochrome, e.g. traffic signals

// lower-power than TFT
// less memory due to CGROM/CGRAM
// cheaper

// Vdd is voltage for logic operation
// V0 is adjustable voltage for LCD display (can be GND for stronger lighting?)


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
  u32 cycles_to_delay = us * cycles_per_us;
  while ((GET_CYCLE_COUNTER() - start_cycles) < cycles_to_delay);
}

// TODO(Ryan): Segger SystemView (https://mcuoneclipse.com/2015/11/16/segger-systemview-realtime-analysis-and-visualization-for-freertos/)

// TODO(Ryan): Understand Systick/RCC more in depth (weeW systick microsecond; matej;)
// TODO(Ryan): Verify clock signal by measuring GPIO pin toggling frequency

typedef struct LCDInfo LCDInfo;
struct LCDInfo
{
  GPIO_TypeDef *gpio_base;
  u32 rs_dio, rw_dio, e_dio;
};

typedef struct LCDInit LCDInit;
struct LCDInit
{
  GPIO_TypeDef *gpio_base;
  // IMPORTANT(Ryan): Data pins must be consecutive
  u16 data_pin_start, rs_pin, rw_pin, e_pin;
}; 

typedef u32 LCD_WRITE_TYPE;
enum
{
  LCD_WRITE_TYPE_DATA,
  LCD_WRITE_TYPE_CMD
};

INTERNAL void
lcd_write(LCDInfo *info, LCD_WRITE_TYPE type, u8 data)
{
  if (type == LCD_WRITE_TYPE_DATA)
  {
    dio_output_set(info->rs_dio, 1);
  }
  else if (type == LCD_WRITE_TYPE_CMD)
  {
    dio_output_set(info->rs_dio, 0);
  }
  else
  {
    return;
  }

  dio_output_set(info->rw_dio, 0);
  delay_us(10);
  dio_output_set(info->e_dio, 1);
  delay_us(5);
  // IMPORTANT(Ryan): This is where pin ordering helps  
  info->gpio_base->ODR &= 0xFF00;
  info->gpio_base->ODR |= data;
  delay_us(10);
  dio_output_set(info->e_dio, 0);
}



INTERNAL LCDInfo
init_lcd1602(LCDInit *init)
{
  LCDInfo result = ZERO_STRUCT;

  u16 data_pin_mask = 0;
  for (u16 data_pin_i = init->data_pin_start; 
       data_pin_i < init->data_pin_start + 8; 
       data_pin_i += 1)
  {
    data_pin_mask |= (1 << data_pin_i);
  }

  GPIO_InitTypeDef gpio_init = ZERO_STRUCT;
  gpio_init.Pin = data_pin_mask;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(init->gpio_base, &gpio_init);

  gpio_init.Pin = init->rs_pin;
  result.rs_dio = dio_add_output(s8_lit("lcd_rs"), &gpio_init, init->gpio_base, 0);

  gpio_init.Pin = init->rw_pin;
  result.rw_dio = dio_add_output(s8_lit("lcd_rw"), &gpio_init, init->gpio_base, 0);

  gpio_init.Pin = init->e_pin;
  result.e_dio = dio_add_output(s8_lit("lcd_e"), &gpio_init, init->gpio_base, 0);

  result.gpio_base = init->gpio_base;
  
  HAL_Delay(20);
  // 8bit, 2lines, 5x11 font size
  lcd_write(&result, LCD_WRITE_TYPE_CMD, 0x3C);
  HAL_Delay(5);
  // on
  lcd_write(&result, LCD_WRITE_TYPE_CMD, 0x0C);
  HAL_Delay(5);
  // clear display
  lcd_write(&result, LCD_WRITE_TYPE_CMD, 0x01);
  HAL_Delay(5);
  // put cursor at start
  lcd_write(&result, LCD_WRITE_TYPE_CMD, 0x02);
  HAL_Delay(5);

  return result;
}
