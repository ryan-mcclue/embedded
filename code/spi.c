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
};

GLOBAL SpiState global_spi_state; 

INTERNAL STATUS
spi_init(void)
{
  STATUS result = STATUS_FAILED;

  // Pulls are only for undriven signals, which most commonly would be inputs. 
  // Outputs are driven

  GPIO_InitTypeDef init = ZERO_STRUCT;
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

  init.Pin = GPIO_PIN_12;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.clk_dio = dio_add_output(s8_lit("spi4_clk"), &init, GPIOE, 0);

  // TODO(Ryan): If not required, don't bother setting up? 
  init.Pin = GPIO_PIN_13;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4; 
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.miso_dio = dio_add_input(s8_lit("spi4_miso"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_14;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  global_spi_state.mosi_dio = dio_add_output(s8_lit("spi4_mosi"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_10;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 backlight_index = dio_add_output(s8_lit("backlight"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_7;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 data_control_index = dio_add_output(s8_lit("data_control"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_8;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 reset_index = dio_add_output(s8_lit("reset"), &init, GPIOE, 0);

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
  // dio_output_set(global_spi_state.nss_dio, 1);

  // There is usually at least one more signal, providing framing, but as SPI is more a defacto than a formal standard, different devices treat this signal differently. 

  // NSS is weird, it's an open collector output that needs a pull up resistor to work properly.
  // Throw a ~5k pull up on it and it'll go
  // If a pullup is switched on for this pin in GPIOx_PUPDR, the pin will be pulled up, but this may take time, depending on the capacitive loading on the pin, given the pullup is relatively weak (nominally 40kΩ). 

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

// https://youtu.be/H_5QQQnP0zg?list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&t=1160
