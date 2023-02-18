// SPDX-License-Identifier: zlib-acknowledgement

// PD8 USART3 TX 
// PD9 USART3 RX

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

void initialise_usart(void)
{
  // gpio clock
  // gpio af

  // note HAL also has _ex files with more pin mappings
  
  __HAL_RCC_USART3_CLK_ENABLE();

  // usart clock (rcc)
  // usart init (.h file look for defines for parameters. 
  // NOTE: for peripheral bases, typically in main .h file
  // sometimes might be nice and mention if parameter is defined for you
  
  // usart interrupt (search for NVIC in hal cortex file)
  // find appropriate ISR name in startup.s file

  USART_InitTypeDef usart_init = ZERO_STRUCT;
  usart_init.BaudRate = 57600;
  usart_init.WordLength = USART_WORDLENGTH_8B;
  usart_init.StopBits = USART_STOPBITS_1;  
  usart_init.Parity = USART_PARITY_NONE;   
  usart_init.Mode = USART_MODE_TX_RX;            
  usart_init.CLKPolarity = USART_POLARITY_LOW;
  usart_init.CLKPhase = USART_PHASE_1EDGE;
  usart_init.CLKLastBit = USART_LASTBIT_DISABLE;

  USART_HandleTypeDef usart_handle = ZERO_STRUCT;
  usart_handle.Instance = USART3;
  usart_handle.Init = usart_init;

  // IMPORTANT(Ryan): Seems that even if buffer size is requested, this is only if we use
  // HAL supplied interrupt methods.
  // The actual buffer of the UART hardware is already set
  // So, can leave this out

  // TODO(Ryan): May have to fiddle/include DMA parameters in init struct if want them?

  HAL_UART_Init

  HAL_NVIC_SetPriority()
  HAL_NVIC_EnableIRQ()
}
