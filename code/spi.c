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

INTERNAL STATUS
spi_test(void)
{
  STATUS result = STATUS_FAILED;

  GPIO_InitTypeDef init = ZERO_STRUCT;
  init.Pin = GPIO_PIN_11;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_PULLUP;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  // IMPORTANT(Ryan): If controlled by software, this will be invert
  // Example transaction: HAL_GPIO_WritePin(low); HAL_SPI_Transmit(); HAL_GPIO_WritePin(high);
  u32 spi4_nss_index = dio_add_output(s8_lit("spi4_nss"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_12;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_clk_index = dio_add_output(s8_lit("spi4_clk"), &init, GPIOE, 0);

  // TODO(Ryan): If not required, don't bother setting up? 
  init.Pin = GPIO_PIN_13;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4; 
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_miso_index = dio_add_input(s8_lit("spi4_miso"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_14;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_mosi_index = dio_add_output(s8_lit("spi4_mosi"), &init, GPIOE, 0);

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

  SPI_HandleTypeDef hspi = ZERO_STRUCT;
  hspi.Instance = SPI4;
  hspi.Init.Mode = SPI_MODE_MASTER; 
  // can make SPI unidirectional, however we want bidirectional
  hspi.Init.Direction = SPI_DIRECTION_2LINES; 
  // dependent on device interacting with: flash device only supports 8bits
  hspi.Init.DataSize = SPI_DATASIZE_8BIT; 
  // dependent on device interacting with
  hspi.Init.CLKPolarity = SPI_POLARITY_LOW; 
  hspi.Init.CLKPhase = SPI_PHASE_1EDGE; 
  // in situations where multiple devices, i.e. multiple NSS signals, would would software
  hspi.Init.NSS = SPI_NSS_HARD_OUTPUT; 
  // this is dependent on SPI peripheral bus clock, e.g. APB 100MHz
  // we know, device can operate up to 60MHz, however this if fine for now
  // better off starting at a low speed to rule out possible noise from long wires on dev board
  hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi.Init.CRCPolynomial = 0;
  if (HAL_SPI_Init(&hspi) == HAL_OK)
  {
    // __HAL_SPI_ENABLE();
    // NVIC_Priority()
    // SPI4_IRQn

    u8 data_tx[4] = {0xde, 0xad, 0xbe, 0xef};
    u8 data_rx[4];

    HAL_SPI_TransmitReceive(&hspi, data_tx, data_rx, 4, 1000);

    result = STATUS_SUCCEEDED;
  }

  return result;
}

void
spi_display(void)
{
  // monochrome displays bit orientated
  
  // IMPORTANT(Ryan): Don't supply power to screen before we know code is correct to avoid damaging screen
}

//  void
//  transmit/recieve/transceive();
// https://youtu.be/H_5QQQnP0zg?list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&t=1160
