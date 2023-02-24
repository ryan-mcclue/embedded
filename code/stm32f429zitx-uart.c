// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): Investigate cortex-m architecture like interrupt registers like PRIMASK

// NOTE(Ryan): No nested interrupts at all
#define ATOMIC_BEGIN() __disable_irq()
#define ATOMIC_END() __enable_irq()

// NOTE(Ryan): Using an exception mask register, allow nested interrupts in the form of NMI and HardFault
GLOBAL volatile u32 global_primask_reg;
#define CRITICAL_BEGIN() \
    do { \
        global_primask_reg = __get_PRIMASK(); \
        __set_PRIMASK(1); \
    } while (0)
#define CRITCAL_END() \
    do { \
        __set_PRIMASK(global_primask_reg); \
    } while (0)


typedef struct UartParams UartParams;
struct UartParams
{
  // NOTE(Ryan): GPIO settings
  // TODO(Ryan): Investigate including pin frequency/speed
  u32 rx_pin, tx_pin; 
  u32 af;
  // IMPORTANT(Ryan): Assume rx and tx pins on same GPIO port
  GPIO_TypeDef *gpio_base; 

  // NOTE(Ryan): UART settings
  // IMPORTANT(Ryan): Assuming 8N1
  u32 baud_rate; 
  USART_TypeDef *uart_base;

  // two interrupt possibilities:
  //   rxne (recieve character ready)
  //   txe (ready to transmit)

  // TODO(Ryan): Consider specifying interrupt priority
  // #define QAD_IRQPRIORITY_UART1 ((uint8_t)0x09)
  /*
   * u32 rx_buf_get_idx;
   * u32 rx_buf_put_idx;
   * u8 *rx_buf;
   * u32 rx_buf_len;
   *
   * u32 tx_buf_get_idx;
   * u32 tx_buf_put_idx;
   * u8 *tx_buf;
   * u32 tx_buf_len;
   *
   * UartStats stats; (a lot of these stats recorded might indicate bad serial connection)
   */
};

typedef struct UartStats UartStats;
struct UartStats
{
  // NOTE(Ryan): Software
  u32 rx_buf_overrun_err;
  u32 tx_buf_overrun_err;

  // NOTE(Ryan): Hardware
  u32 rx_overrun_err;
  u32 rx_noise_err;
  u32 rx_frame_err;
  u32 rx_parity_err;
};

// TODO(Ryan): Consider changing name to UartState so as to record if active/deactive
typedef struct UartResult UartResult;
struct UartResult
{
  UART_HandleTypeDef handle;
  STATUS status;
};

INTERNAL UartResult 
stm32f429zitx_initialise_uart(UartParams *uart_params)
{
  UartResult result = ZERO_STRUCT;

  ASSERT(__HAL_RCC_GPIOD_IS_CLK_ENABLED());

  // NOTE(Ryan): GPIO init
  GPIO_InitTypeDef gpio_init = ZERO_STRUCT; 
  gpio_init.Pin = uart_params->tx_pin;
  gpio_init.Mode = GPIO_MODE_AF_PP; // almost always PP as want to be able to set 0 and 1
  gpio_init.Pull = GPIO_PULLUP; // only want if reading
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = uart_params->af;
	HAL_GPIO_Init(uart_params->gpio_base, &gpio_init);

	gpio_init.Pin = uart_params->rx_pin;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	// prevent possible spurious reads of a start bit resulting from floating state
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init.Alternate = uart_params->af;
	HAL_GPIO_Init(uart_params->gpio_base, &gpio_init);


  // NOTE(Ryan): USART init
  // only clock is USART specific
  ASSERT(uart_params->uart_base == USART3);
  __HAL_RCC_USART3_CLK_ENABLE();

  // IMPORTANT(Ryan): Even though USART peripheral, use UART struct
  // This is because we don't want an outgoing clock signal
  // This will cause us to read garbage on receive for most serial terminals 
  UART_InitTypeDef uart_init = ZERO_STRUCT;
  uart_init.BaudRate = uart_params->baud_rate;
  uart_init.WordLength = UART_WORDLENGTH_8B;
  uart_init.StopBits = UART_STOPBITS_1;
  uart_init.Parity = UART_PARITY_NONE;
  uart_init.Mode = UART_MODE_TX_RX;
	uart_init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_init.OverSampling = UART_OVERSAMPLING_16;

  result.handle.Instance = uart_params->uart_base;
  result.handle.Init = uart_init;

  // IMPORTANT(Ryan): 
  // If we want to process multibyte data, will require interrupts to avoid buffer overrun in polling.
  // This is because of UART hardware having a 1 byte buffer

  // TODO(Ryan): May have to fiddle/include DMA parameters in init struct if want them?

  HAL_StatusTypeDef hal_status = HAL_UART_Init(&result.handle);
  if (hal_status != HAL_OK)
  {
    result.status = STATUS_FAILED; 
  }
  else
  {
    // TODO(Ryan): Investigate savings gained by lazy loading interrupts

    // keep in mind for interupt:
    //   * volatile if global variable
    //   * atomic/disable interrupts during section 
    //   * consider interrupt priorities down the road?

    // __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
    // __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

    // IPR registers store 4 8-bit priority fields 
    // Within priority field, further sub-divided into group and sub-priority level 
    // (the possible subdivisions/groupings dependant on priority groupings available on specific architecture)
    // So, an interrupt priority in an IPR register and specific field
    // This used to determine preemption

    // lower priority value higher priority? so this is highest priority?
    // NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    // NVIC_EnableIRQ(USART3_IRQn);
    
    result.status = STATUS_SUCCEEDED;
  }

  return result;
}

#if 0
void
USART3_IRQHandler(void)
{
  console_interrupt_handler();
}

//  the UART does continuously generate interrupts once the receive buffer is full - even when I stopped typing characters into PuTTY. 
//    To avoid this, I added the function LL_USART_RequestRxDataFlush() 
//    (disable interrupts in RXNE is recieve buffer is full)

INTERNAL void 
console_interrupt_handler(void)
{
  u32 status_register = global_console.uart_handle->Instance->SR;

  CRITICAL_BEGIN();

  // NOTE(Ryan): Put into rx buffer
  if (status_register & UART_FLAG_RXNE) 
  {
    u32 next_rx_write_index = global_console.rx_buf_write_index + 1;
    if (next_rx_write_index >= global_console.rx_buf_len)
    {
      next_rx_write_index = 0;
    }

    if (next_rx_write_index == global_console.rx_buf_read_index)
    {
      // Need to read DR.
      INC_SATURATE_U16(global_console.stats.rx_buf_overrun_err);
    } 
    else 
    {
      global_console.rx_buf[global_console.rx_buf_write_index] = global_console.uart_handle->Instance->DR;
      global_console.rx_buf_write_index = next_rx_write_index;
    }
  }

  // NOTE(Ryan): Take from tx buffer
  if (status_register & UART_FLAG_TXE) 
  {
    if (global_console.tx_buf_read_index == global_console.tx_buf_write_index)
    {
      // No characters to send, disable the interrrupt.
      LL_USART_DisableIT_TXE(global_console.uart_reg_base);
    } 
    else 
    {
      global_console.uart_handle->Instance->TX = global_console.tx_buf[global_console.tx_buf_read_index];
      if (global_console.tx_buf_read_index < (global_console.tx_buf_len - 1))
      {
        global_console.tx_buf_read_index++;
      }
      else
      {
        global_console.tx_buf_read_index = 0;
      }
    }
  }

  if (status_register & (UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE | UART_FLAG_PE)) 
  {
    // Just reading the data register clears the error bits.
    // IMPORTANT(Ryan): This seems common for most UARTS
    (void)global_console.uart_handle->Instance->DR;

    if (status_register & UART_FLAG_ORE)
    {
      INC_SATURATE_U16(global_console.stats.rx_overrun_err);
    }
    else if (status_register & UART_FLAG_NE)
    {
      INC_SATURATE_U16(global_console.stats.rx_noise_err);
    }
    else if (status_register & UART_FLAG_FE)
    {
      INC_SATURATE_U16(global_console.stats.rx_frame_err);
    }
    else if (status_register & UART_FLAG_PE)
    {
      INC_SATURATE_U16(global_console.stats.rx_parity_err);
    }
  }
  
  CRITICAL_END();
}

INTERNAL void 
console_write_ch(Console *console, char ch)
{
  // put critical in if interrupt reading from
  CRITICAL_BEGIN();

  u32 next_write_index = console->tx_buf_write_index + 1;
  if (next_write_index >= console->tx_buf_len)
  {
    next_write_index = 0;
  }

  if (next_write_index == console->tx_buf_read_index) 
  {
    INC_SATURATE_U16(console->stats.tx_buf_overrun_err);
    CRITICAL_END();
    return;
  }

  console->tx_buf[console->tx_buf_write_index] = ch;
  console->tx_buf_write_index = next_write_index;

  // Ensure the TX interrupt is enabled.
  if (ttys_states[instance_id].uart_reg_base != NULL) {
    LL_USART_EnableIT_TXE(st->uart_reg_base);
  }

  CRITICAL_END();
}

INTERNAL char
console_read_ch(Console *console, char ch)
{
  char result = 0;

  CRITICAL_BEGIN();

  if (console->rx_buf_read_index == console->rx_buf_write_index)
  {
    CRITICAL_END();
    return result;
  }

  u32 next_read_index = console->rx_buf_read_index + 1;
  if (next_read_index >= console.rx_buf_len)
  {
    next_read_index = 0;
  }

  result = console->rx_buf[console->rx_buf_read_index];
  console->rx_buf_read_index = next_read_index;

  CRITICAL_END();

  return result;
}
#endif
