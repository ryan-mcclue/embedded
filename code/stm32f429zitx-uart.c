// SPDX-License-Identifier: zlib-acknowledgement


#if 0
// TODO(Ryan): Make distinctions between what you define, and what is typically defined by HAL
#define QAD_UART1_TX_PORT GPIOA
#define QAD_UART1_TX_PIN GPIO_PIN_9
#define QAD_UART1_TX_AF GPIO_AF7_USART1
#define QAD_UART1_RX_PORT GPIOA
#define QAD_UART1_RX_PIN GPIO_PIN_10
#define QAD_UART1_RX_AF GPIO_AF7_USART1
#define QAD_UART1_BAUDRATE 57600
#define QAD_UART1_TX_FIFOSIZE 256
#define QAD_UART1_RX_FIFOSIZE 256
// higher number, lower priority
#define QAD_IRQPRIORITY_UART1 ((uint8_t)0x09)

// TODO(Ryan): Investigate cortex-m architecture like interrupt registers like PRIMASK

// TODO(Ryan): So, no nested here?
// Critical region start/end macros, when it is known that you are running at
// the base level.
#define CRIT_START() __disable_irq()
#define CRIT_END() __enable_irq()

// TODO(Ryan): So, allows nesting, i.e. NMI and HardFault
// Critical region start/end macros, which work regardless of where you are
// runing in a handler, or at the base level.
#define CRIT_STATE_VAR uint32_t _primask_save
#define CRIT_BEGIN_NEST()                       \
    do {                                        \
        _primask_save = __get_PRIMASK();        \
        __set_PRIMASK(1);                       \
    } while (0)
#define CRIT_END_NEST()                         \
    do {                                        \
        __set_PRIMASK(_primask_save);           \
    } while (0)



void USART1_IRQHandler(void)
{
  usart_interrupt(usart_instance_1, USART1_IRQn);

UART_HandleTypeDef *handle = &uart->getHandle();

  if (__HAL_UART_GET_FLAG(handle, UART_FLAG_RXNE)) {
    uint8_t data = uart->data_rx();
    if (rx_state) {
    	rx_fifo->push(data);
    	// clear this to ensure it doesn't continually call the interrupt handler again
    	__HAL_UART_CLEAR_FLAG(handle, UART_FLAG_RXNE);
    }
  }

  if (__HAL_UART_GET_FLAG(handle, UART_FLAG_TXE)) {
  	if (!tx_fifo->empty()) {
      uart->data_tx(tx_fifo->pop());
  	} else {
  		uart->stop_tx();
  		tx_state = PeriphInactive;
  	}

  	__HAL_UART_CLEAR_FLAG(handle, UART_FLAG_TXE);



}

void usart_interrupt(instance_id, irq_type)
{
  UsartState state = global_usart_state[instance_id];
}
#endif

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

    // __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
    // __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

    // priorities and sub group priorities ...
    // so instead of major.minor interrupt priorities,
    // only have single number between 0-15
    // HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);


    // USART3_IRQn
    // NVIC_SetPriority(irq_type,
    //                  NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    // NVIC_EnableIRQ(irq_type);
    result.status = STATUS_SUCEEDED;
  }

  return result;
}




#if 0

static void ttys_interrupt(enum ttys_instance_id instance_id,
                           IRQn_Type irq_type)
{
    struct ttys_state* st;
    uint8_t sr;
    CRIT_STATE_VAR;

    if (instance_id >= TTYS_NUM_INSTANCES)
        return;

    st = &ttys_states[instance_id];

    // If instance is not open, we should not get an interrupt, but for safety
    // just disable it.
    if (st->uart_reg_base == NULL) {
        NVIC_DisableIRQ(irq_type);
        return;
    }

    sr = st->uart_reg_base->STATUS_REG;

    CRIT_BEGIN_NEST();
    if (sr & RXNE_BIT_MASK) {
        // Got an incoming character.
        uint16_t next_rx_put_idx = st->rx_buf_put_idx + 1;
        if (next_rx_put_idx >= TTYS_RX_BUF_SIZE)
            next_rx_put_idx = 0;
        if (next_rx_put_idx == st->rx_buf_get_idx) {
            // Need to read DR.
            INC_SAT_U16(cnts_u16[CNT_RX_BUF_OVERRUN]);
        } else {
            st->rx_buf[st->rx_buf_put_idx] = st->uart_reg_base->DATA_RX_REG;
            st->rx_buf_put_idx = next_rx_put_idx;
        }
    }
    if (sr & TXE_BIT_MASK) {
        // Can send a character.
        if (st->tx_buf_get_idx == st->tx_buf_put_idx) {
            // No characters to send, disable the interrrupt.
            LL_USART_DisableIT_TXE(st->uart_reg_base);
        } else {
            st->uart_reg_base->DATA_TX_REG = st->tx_buf[st->tx_buf_get_idx];
            if (st->tx_buf_get_idx < TTYS_TX_BUF_SIZE-1)
                st->tx_buf_get_idx++;
            else
                st->tx_buf_get_idx = 0;
        }
    }

    if (sr & (ORE_BIT_MASK | NE_BIT_MASK | FE_BIT_MASK | PE_BIT_MASK)) {
        // Error bits(s) detected. First clear them out.

#if CONFIG_USART_TYPE == 1
        // Just reading the data register clears the error bits.
        (void)st->uart_reg_base->DR;
#elif (CONFIG_USART_TYPE == 2 || CONFIG_USART_TYPE == 3)
        // Writing the error bits to the ICR clears them.
        st->uart_reg_base->ICR = sr & 0xf;
#endif

        // Record the error(s).
        if (sr & ORE_BIT_MASK)
            INC_SAT_U16(cnts_u16[CNT_RX_UART_ORE]);
        if (sr & NE_BIT_MASK)
            INC_SAT_U16(cnts_u16[CNT_RX_UART_NE]);
        if (sr & FE_BIT_MASK)
            INC_SAT_U16(cnts_u16[CNT_RX_UART_FE]);
        if (sr & PE_BIT_MASK)
            INC_SAT_U16(cnts_u16[CNT_RX_UART_PE]);

#if CONFIG_USART_TYPE == 2
        // TODO Needed?
        sr = st->uart_reg_base->ISR;
#endif

    }
    CRIT_END_NEST();
}


int32_t ttys_putc(enum ttys_instance_id instance_id, char c)
{
    struct ttys_state* st;
    uint16_t next_put_idx;
    CRIT_STATE_VAR;

    if (instance_id >= TTYS_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;
    st = &ttys_states[instance_id];

    // Calculate the new TX buffer put index
    CRIT_BEGIN_NEST();
    next_put_idx = st->tx_buf_put_idx + 1;
    if (next_put_idx >= TTYS_TX_BUF_SIZE)
        next_put_idx = 0;

    // If buffer is full, then return error.
    while (next_put_idx == st->tx_buf_get_idx) {
        INC_SAT_U16(cnts_u16[CNT_TX_BUF_OVERRUN]);
        CRIT_END_NEST();
        return MOD_ERR_BUF_OVERRUN;
    }

    // Put the char in the TX buffer.
    st->tx_buf[st->tx_buf_put_idx] = c;
    st->tx_buf_put_idx = next_put_idx;

    // Ensure the TX interrupt is enabled.
    if (ttys_states[instance_id].uart_reg_base != NULL) {
        LL_USART_EnableIT_TXE(st->uart_reg_base);
    }
    CRIT_END_NEST();
    return 0;
}

/*
 * @brief Get a received character.
 *
 * @param[in] instance_id Identifies the ttys instance.
 * @param[out] c Received character.
 *
 * @return Number of characters returned (0 or 1)
 */
int32_t ttys_getc(enum ttys_instance_id instance_id, char* c)
{
    struct ttys_state* st;
    int32_t next_get_idx;
    CRIT_STATE_VAR;

    if (instance_id >= TTYS_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;

    st = &ttys_states[instance_id];

    // Check if buffer is empty.
    CRIT_BEGIN_NEST();
    if (st->rx_buf_get_idx == st->rx_buf_put_idx) {
        CRIT_END_NEST();
        return 0;
    }

    // Get a character and advance get index.
    next_get_idx = st->rx_buf_get_idx + 1;
    if (next_get_idx >= TTYS_RX_BUF_SIZE)
        next_get_idx = 0;
    *c = st->rx_buf[st->rx_buf_get_idx];
    st->rx_buf_get_idx = next_get_idx;
    CRIT_END_NEST();
    return 1;
}
#endif
