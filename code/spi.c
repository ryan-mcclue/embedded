// SPDX-License-Identifier: zlib-acknowledgement

// NOR flash is cheaper and slower than NAND.
// often NOR uses SPI, QSPI instead of parallel interfaces

// QSPI uses 4 data lines instead of 2 (quad-spi, i.e. queued spi)
// I2S closely related to SPI for audio. Typically share pins?

// debouncing in software although more involved than a hardware capacitor, allows for say holding/releasing button for x amount of time

// select pin duplicates for each slave

// seems various configuration options for SPI, e.g. polarity, 8/16bit etc.

// bidirectional, and no ACK bytes
// clock will pulse for sending bytes, e.g. a group of pulses equates to bytes being transferred
// (so four groups, four bytes)

// IMPORTANT(Ryan): When viewing something digital like SPI on oscilloscope,
// can see noise, e.g. MISO noise from clock signal

// common for SPI devices to have pass-through holes to easily connect oscilloscope to?

// send via polling (DR register and busy flag), check signal, then do interrupt?

// TODO(Ryan): delay and roll functionality for oscilloscope to slowly watch signal progress

typedef struct SpiState SpiState;
struct SpiState
{
  SPI_HandleTypeDef handle;
  u32 nss_dio, clk_dio, mosi_dio, miso_dio;
  u32 rst_dio, dc_dio, backlight_dio;
};

GLOBAL SpiState global_spi_state; 

INTERNAL void
spi_tx(SPI_HandleTypeDef *spi_handle, u8 *data, u16 data_len, u32 timeout)
{
  HAL_SPI_Transmit(spi_handle, data, data_len, timeout);
  __HAL_SPI_DISABLE(spi_handle);
}

INTERNAL STATUS
spi_init(void)
{
  STATUS result = STATUS_FAILED;

  GPIO_InitTypeDef init = ZERO_STRUCT;
  GPIO_InitTypeDef zeroed_init = ZERO_STRUCT;
  
  init.Pin = GPIO_PIN_8;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.rst_dio = dio_add_output(s8_lit("reset"), &init, GPIOE, 1);
  // NOTE(Ryan): Generate reset pulse
  dio_output_set(global_spi_state.rst_dio, 0);
  HAL_Delay(10);
  dio_output_set(global_spi_state.rst_dio, 1);
  HAL_Delay(10);
  dio_output_set(global_spi_state.rst_dio, 0);

  /*
  init = zeroed_init;
  init.Pin = GPIO_PIN_10;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH; // TODO(Ryan): Lowering frequency reduces power
  global_spi_state.backlight_dio = dio_add_output(s8_lit("backlight"), &init, GPIOE, 0);
  // NOTE(Ryan): Test backlight
  dio_output_set(global_spi_state.backlight_dio, 1);
  HAL_Delay(2000);
  dio_output_set(global_spi_state.backlight_dio, 0);
  */

  init = zeroed_init;
  init.Pin = GPIO_PIN_7;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.dc_dio = dio_add_output(s8_lit("data_control"), &init, GPIOE, 0);


  // Pulls are only for undriven signals, which most commonly would be inputs. 
  // Outputs are driven

  init = zeroed_init;
  init.Pin = GPIO_PIN_11;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_PULLUP;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  // IMPORTANT(Ryan): NSS is not literal chip select equivalent
  // It will be set to low if in master mode as long as SPI enabled.
  // Will go high if SPI disabled.
  // Actual chip select would be GPIO controlled
  // IMPORTANT(Ryan): Not only errata important, but also check pin mapping correspondance
  global_spi_state.nss_dio = dio_add_output(s8_lit("spi4_nss"), &init, GPIOE, 0);

  // __HAL_SPI_DISABLE

  init = zeroed_init;
  init.Pin = GPIO_PIN_12;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.clk_dio = dio_add_output(s8_lit("spi4_clk"), &init, GPIOE, 0);

  /*
  init = zeroed_init;
  // TODO(Ryan): If not required, don't bother setting up? 
  init.Pin = GPIO_PIN_13;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4; 
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.miso_dio = dio_add_input(s8_lit("spi4_miso"), &init, GPIOE, 0);
  */

  init = zeroed_init;
  init.Pin = GPIO_PIN_14;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.mosi_dio = dio_add_output(s8_lit("spi4_mosi"), &init, GPIOE, 0);




  __HAL_RCC_SPI4_CLK_ENABLE();
  // TODO(Ryan): Necessary?
  // __HAL_RCC_SPI4_FORCE_RESET();
  // __HAL_RCC_SPI4_RELEASE_RESET();

  global_spi_state.handle.Instance = SPI4;
  global_spi_state.handle.Init.Mode = SPI_MODE_MASTER; 

  // can make SPI unidirectional, however we want bidirectional
  global_spi_state.handle.Init.Direction = SPI_DIRECTION_2LINES; 
  // dependent on device interacting with: flash device only supports 8bits
  global_spi_state.handle.Init.DataSize = SPI_DATASIZE_8BIT; 
  // dependent on device interacting with
  global_spi_state.handle.Init.CLKPolarity = SPI_POLARITY_LOW; 
  global_spi_state.handle.Init.CLKPhase = SPI_PHASE_1EDGE; 
  // in situations where multiple devices, i.e. multiple NSS signals, would software
  global_spi_state.handle.Init.NSS = SPI_NSS_HARD_OUTPUT; 
  // this is dependent on SPI peripheral bus clock, e.g. APB 100MHz
  // we know, device can operate up to 60MHz, however this if fine for now
  // better off starting at a low speed to rule out possible noise from long wires on dev board
  global_spi_state.handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  global_spi_state.handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  global_spi_state.handle.Init.TIMode = SPI_TIMODE_DISABLE;
  global_spi_state.handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  global_spi_state.handle.Init.CRCPolynomial = 0;
  if (HAL_SPI_Init(&global_spi_state.handle) == HAL_OK)
  {
    result = STATUS_SUCCEEDED;
  }

  // There is usually at least one more signal, providing framing, but as SPI is more a defacto than a formal standard, different devices treat this signal differently. 

  // NSS is weird, it's an open collector output that needs a pull up resistor to work properly.
  // Throw a ~5k pull up on it and it'll go
  // If a pullup is switched on for this pin in GPIOx_PUPDR, the pin will be pulled up, but this may take time, depending on the capacitive loading on the pin, given the pullup is relatively weak (nominally 40kΩ). 

  dio_output_set(global_spi_state.dc_dio, 0);
  
  u8 on_horizontal_extended = 0x21; 
  spi_tx(&global_spi_state.handle, &on_horizontal_extended, 1, 1000);

  // setting higher voltage gives greater contrast
  u8 voltage = 0xF0; 
  spi_tx(&global_spi_state.handle, &voltage, 1, 1000);

  u8 basic = 0x20; 
  spi_tx(&global_spi_state.handle, &basic, 1, 1000);

  u8 normal = 0x0C; 
  spi_tx(&global_spi_state.handle, &normal, 1, 1000);

  
  dio_output_set(global_spi_state.dc_dio, 1);
  u8 val = 0x1F;
  spi_tx(&global_spi_state.handle, &val, 1, 1000);
  val = 0x05;
  spi_tx(&global_spi_state.handle, &val, 1, 1000);
  val = 0x07;
  spi_tx(&global_spi_state.handle, &val, 1, 1000);
  val = 0x00;
  spi_tx(&global_spi_state.handle, &val, 1, 1000);
  spi_tx(&global_spi_state.handle, &val, 1, 1000);


  return result;
}


INTERNAL void
spi_test(void)
{
  // CMSIS SVD (system view description) file is XML
  // typically contained in a ZIP .pack files 
  // Good place for .pack files from Keil MDK5: https://www.keil.com/dd2/pack/ 

  u8 data_tx[4] = {0xde, 0xad, 0xbe, 0xef};

  HAL_SPI_Transmit(&global_spi_state.handle, data_tx, 4, 1000);

  // IMPORTANT(Ryan): Required for NSS
  // TODO(Ryan): Overcome lag in signal by manually controlling with GPIO
  __HAL_SPI_DISABLE(&global_spi_state.handle);

  // IMPORTANT(Ryan): Don't supply power to screen before we know code is correct to avoid damaging screen
  // monochrome displays bit orientated
  // bus timing frequency (4MHz max.)
  // instruction set (example instruction)
  // initialisation procedure (res pulse at least 100ms after Vdd)
  

}

void
spi_timings(void)
{
  // polling/dma/interrupt mode

  // e.g:
  // 20 clear screens: 5000ms
  // 30k vertical lines:
  // 5k characters: 

}

void spi_write()
{
  // ensure we don't start a new DMA transmission before last one has finished
  GLOBAL global_display_busy_dma = 0;

  // TODO(Ryan):
  // PWM pin for dimming backlight?
  // turn backlight off when no new data being written to it?

  // however, setting and starting many small DMA transactions takes time (so much so, could be slower than polling)
  // so, throughput important for DMA
  // IMPORTANT(Ryan): So, for short data, polling mode is actually faster than DMA (i.e. only faster if CPU can do other work)
  // we see this by adding a trigger GPIO pin for view on logic analyser an many time intervals between transactions
  // so, buffer SPI data (we don't need a full 1:1 mapping of screen to buffer size to get benefits). 
  // However, will have to implement double buffering so the MCU does not overwrite buffer as DMA is sending it to SPI
  // IMPORTANT(Ryan): If just measuring one thing, e.g. clear screen, measure time in logic analyser?
  // also relevent to timing is clock speed of MCU and SPI speed
  while (global_display_busy_dma) {}
  global_display_busy_dma = 1;
  if (data_size < DMA_CUTOFF)
  {
    SPI_Transmit();
  }
  else
  {
    SPI_Transmit_DMA();
  }
}

void
spi_tx_callback_int(void)
{
  global_display_busy_dma = 0;
}
