// SPDX-License-Identifier: zlib-acknowledgement

// https://www.youtube.com/watch?v=TUD4qnQjSzs&list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&index=28&t=713s

// reflective layers cost more, but can be lower power as less backlight required (however only effective in well-lit areas)

// dot matrix monochrome, e.g. traffic signals

// lower-power than TFT
// less memory due to CGROM/CGRAM (DDRAM for LCD is where displayed characters go)
// cheaper

// Vdd is voltage for logic operation
// V0 is adjustable voltage for LCD display (can be GND for stronger lighting?)

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
lcd_write(LCDInfo *info, LCD_WRITE_TYPE type, u8 data, b32 four_bits)
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
  if (four_bits)
  {
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= (data & 0xf0);
    delay_us(10);
    dio_output_set(info->e_dio, 0);
    delay_us(20);

    dio_output_set(info->e_dio, 1);
    delay_us(5);
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= ((data << 4) & 0xf0);
    delay_us(10);
    dio_output_set(info->e_dio, 0);
  }
  else
  {
    info->gpio_base->ODR &= 0xFF00;
    info->gpio_base->ODR |= data;
    delay_us(10);
    dio_output_set(info->e_dio, 0);
  }
}

INTERNAL void
lcd_s8(LCDInfo *info, u32 line_num, u32 col_num, String8 str)
{
  u32 pos = 0; 
  if (line_num == 1)
  {
    pos = 0x40;
  }

  lcd_write(info, LCD_WRITE_TYPE_CMD, 0x80 + pos + col_num);

  for (u32 i = 0; i < str.size; i += 1)
  {
    lcd_write(info, LCD_WRITE_TYPE_DATA, str.str[i]);
    delay_us(100);
  }
}

INTERNAL LCDInfo
init_lcd1602(LCDInit *init, b32 four_bit)
{
  LCDInfo result = ZERO_STRUCT;

  // TODO(Ryan): Change to 4-7
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

  if (four_bit)
  {
    // this is the initialisation phase that goes in both
    HAL_Delay(20);
    dio_output_set(info->rs_dio, 0);
    dio_output_set(info->rw_dio, 0);
    delay_us(10);
    dio_output_set(info->e_dio, 1);
    delay_us(5);
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= 0x30;
    delay_us(10);
    dio_output_set(info->e_dio, 0);

    HAL_Delay(10);
    dio_output_set(info->rs_dio, 0);
    dio_output_set(info->rw_dio, 0);
    delay_us(10);
    dio_output_set(info->e_dio, 1);
    delay_us(5);
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= 0x30;
    delay_us(10);
    dio_output_set(info->e_dio, 0);

    HAL_Delay(1);
    dio_output_set(info->rs_dio, 0);
    dio_output_set(info->rw_dio, 0);
    delay_us(10);
    dio_output_set(info->e_dio, 1);
    delay_us(5);
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= 0x30;
    delay_us(10);
    dio_output_set(info->e_dio, 0);

    HAL_Delay(1);
    dio_output_set(info->rs_dio, 0);
    dio_output_set(info->rw_dio, 0);
    delay_us(10);
    dio_output_set(info->e_dio, 1);
    delay_us(5);
    info->gpio_base->ODR &= 0xFF0F;
    info->gpio_base->ODR |= 0x20; // 4-bit mode
    delay_us(10);
    dio_output_set(info->e_dio, 0);

    // now go on and enable 4 bit communication instruction

  }
  else
  {
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
  }

  lcd_s8(&result, 0, 0, s8_lit("Ryanx"));
  delay_us(100);
  lcd_s8(&result, 1, 0, s8_lit("McClue"));

  return result;
}
