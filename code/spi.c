// SPDX-License-Identifier: zlib-acknowledgement

// NOR flash is cheaper and slower than NAND.
// often NOR uses SPI, QSPI instead of parallel interfaces

// QSPI uses 4 data lines instead of 2 (quad-spi, i.e. queued spi)
// I2S closely related to SPI for audio

// debouncing in software although more involved than a hardware capacitor, allows for say holding/releasing button for x amount of time

// select pin duplicates for each slave

// seems various configuration options for SPI, e.g. polarity, 8/16bit etc.

// bidirectional, and no ACK bytes
// clock will pulse for sending bytes, e.g. a group of pulses equates to bytes being transferred
// (so four groups, four bytes)

// IMPORTANT(Ryan): When viewing something digital like SPI on oscilloscope,
// can see noise, e.g. MISO noise from clock signal

// common for SPI devices to have pass-through holes to easily connect oscilloscope to?

// TODO(Ryan): delay and roll functionality for oscilloscope to slowly watch signal progress

void
spi_init(void)
{
  GPIO_InitTypeDef init = ZERO_STRUCT;
  init.Pin = GPIO_PIN_11;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_PULLUP;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  // TODO(Ryan): invert?
  u32 spi4_nss_index = dio_add_output(s8_lit("spi4_nss"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_12;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_clk_index = dio_add_output(s8_lit("spi4_clk"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_13;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4; // input?
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_miso_index = dio_add_output(s8_lit("spi4_miso"), &init, GPIOE, 0);

  init.Pin = GPIO_PIN_14;
  init.Mode = GPIO_MODE_AF_PP;
  init.Alternate = GPIO_AF5_SPI4;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 spi4_mosi_index = dio_add_output(s8_lit("spi4_mosi"), &init, GPIOE, 0);

  SPI_HandleTypeDef hspi = ZERO_STRUCT;
  hspi.Instance = SPI4;
  hspi.Init.Mode = SPI_MODE_MASTER; 
  // can make SPI unidirectional, however we want bidirectional
  hspi.Init.Direction = SPI_DIRECTION_2LINES; 
  // dependent on device interacting with: flash device only supports 8bits
  hspi.Init.DataSize = SPI_DATASIZE_8BIT; 
  // dependent on device interacting with
  hspi.Init.CLKPolarity = SPI_POLARITY_LOw; 
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
  HAL_SPI_Init(&hspi);

  u8 data_tx[4] = {0xde, 0xad, 0xbe, 0xef};
  u8 data_rx[4];

  HAL_SPI_TransmitRecieve(&handle, data_tx, data_rx, 4, 1000);
}
// https://youtu.be/RXYA6gVTHrA?t=4192 
// https://youtu.be/uib4Zbdr-1A?list=PLtVUYRe-Z-mcjXXFBte61L8SjyI377VNq&t=1193
