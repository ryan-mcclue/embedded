// SPDX-License-Identifier: zlib-acknowledgement

#include "stm32f429zitx-console.h"

GLOBAL Console global_console;

INTERNAL STATUS 
stm32f429zitx_create_console(MemArena *perm_arena, u32 cmd_str_buf_len, UartParams *uart_params)
{
  STATUS result = STATUS_FAILED;

  global_console.perm_arena = perm_arena;
  global_console.log_active = true;
  global_console.log_level = LOG_LEVEL_VERBOSE;

  global_console.cmd_str_buf_len = cmd_str_buf_len;
  global_console.cmd_str_buf_cursor = 0;
  global_console.cmd_str.str = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, u8, cmd_str_buf_len);
  global_console.cmd_str.size = 0;

  ASSERT(__HAL_RCC_GPIOD_IS_CLK_ENABLED());

  // NOTE(Ryan): GPIO init
  GPIO_InitTypeDef gpio_init = ZERO_STRUCT; 
  gpio_init.Pin = uart_params->tx_pin;
  // NOTE(Ryan): Almost always PP as want to be able to set 0 and 1
  gpio_init.Mode = GPIO_MODE_AF_PP; 
  // NOTE(Ryan): Really only necessary if reading 
  gpio_init.Pull = GPIO_PULLUP; 
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = uart_params->af;
	HAL_GPIO_Init(uart_params->gpio_base, &gpio_init);

	gpio_init.Pin = uart_params->rx_pin;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	// NOTE(Ryan): Prevent possible spurious reads resulting from floating state
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_init.Alternate = uart_params->af;
	HAL_GPIO_Init(uart_params->gpio_base, &gpio_init);


  // NOTE(Ryan): USART init
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

  global_console.uart_info.handle.Instance = uart_params->uart_base;
  global_console.uart_info.handle.Init = uart_init;

  // TODO(Ryan): May have to fiddle/include DMA parameters in init struct if want them?

  HAL_StatusTypeDef hal_status = HAL_UART_Init(&global_console.uart_info.handle);
  if (hal_status != HAL_OK)
  {
    result = STATUS_FAILED; 
  }
  else
  {
    global_console.uart_info.tx_buf_len = uart_params->tx_buf_len;
    global_console.uart_info.tx_buf = MEM_ARENA_PUSH_ARRAY_ZERO(global_console.perm_arena, u8, global_console.uart_info.tx_buf_len);

    global_console.uart_info.rx_buf_len = uart_params->rx_buf_len;
    global_console.uart_info.rx_buf = MEM_ARENA_PUSH_ARRAY_ZERO(global_console.perm_arena, u8, global_console.uart_info.rx_buf_len);

    // TODO(Ryan): Investigate savings gained by lazy loading interrupts
    __HAL_UART_ENABLE_IT(&global_console.uart_info.handle, UART_IT_TXE);
    __HAL_UART_ENABLE_IT(&global_console.uart_info.handle, UART_IT_RXNE);

    // TODO(Ryan): Consider if this priority is appropriate in the grand scheme of things
    // Probably has to be as need to service UART as fast as possible to not get rx overrun errors
    NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(USART3_IRQn);
    
    result = STATUS_SUCCEEDED;
  }

  return result;
}

//  the UART does continuously generate interrupts once the receive buffer is full - even when I stopped typing characters into PuTTY. 
//    To avoid this, I added the function LL_USART_RequestRxDataFlush() 
//    (disable interrupts in RXNE is recieve buffer is full)

// IMPORTANT(Ryan): Buffering important to prevent blocking and possible loss of characters

INTERNAL void 
console_interrupt_handler(void)
{
  u32 status_register = global_console.uart_info.handle.Instance->SR;

  CRITICAL_BEGIN();

  // NOTE(Ryan): Put into rx buffer
  if (status_register & UART_FLAG_RXNE)
  {
    u32 next_rx_write_index = global_console.uart_info.rx_buf_write_index + 1;
    if (next_rx_write_index >= global_console.uart_info.rx_buf_len)
    {
      next_rx_write_index = 0;
    }

    if (next_rx_write_index == global_console.uart_info.rx_buf_read_index)
    {
      // Need to read DR.
      INC_SATURATE_U16(global_console.uart_info.stats.rx_buf_overrun_err);
    } 
    else 
    {
      global_console.uart_info.rx_buf[global_console.uart_info.rx_buf_write_index] = (u8)global_console.uart_info.handle.Instance->DR;
      global_console.uart_info.rx_buf_write_index = next_rx_write_index;
    }
  }

  // NOTE(Ryan): Take from tx buffer
  if (status_register & UART_FLAG_TXE)
  {
    if (global_console.uart_info.tx_buf_read_index == global_console.uart_info.tx_buf_write_index)
    {
      __HAL_UART_DISABLE_IT(&global_console.uart_info.handle, UART_IT_TXE);
    } 
    else 
    {
      global_console.uart_info.handle.Instance->DR = global_console.uart_info.tx_buf[global_console.uart_info.tx_buf_read_index];
      if (global_console.uart_info.tx_buf_read_index < (global_console.uart_info.tx_buf_len - 1))
      {
        global_console.uart_info.tx_buf_read_index++;
      }
      else
      {
        global_console.uart_info.tx_buf_read_index = 0;
      }
    }
  }

  if (status_register & (UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE | UART_FLAG_PE)) 
  {
    // Just reading the data register clears the error bits.
    // IMPORTANT(Ryan): This seems common for most UARTS
    (void)global_console.uart_info.handle.Instance->DR;

    if (status_register & UART_FLAG_ORE)
    {
      INC_SATURATE_U16(global_console.uart_info.stats.rx_overrun_err);
    }
    else if (status_register & UART_FLAG_NE)
    {
      INC_SATURATE_U16(global_console.uart_info.stats.rx_noise_err);
    }
    else if (status_register & UART_FLAG_FE)
    {
      INC_SATURATE_U16(global_console.uart_info.stats.rx_frame_err);
    }
    else if (status_register & UART_FLAG_PE)
    {
      INC_SATURATE_U16(global_console.uart_info.stats.rx_parity_err);
    }
  }
  
  CRITICAL_END();
}

void
USART3_IRQHandler(void)
{
  console_interrupt_handler();
}

INTERNAL void 
console_write_ch(char ch)
{
  CRITICAL_BEGIN();

  u32 next_write_index = global_console.uart_info.tx_buf_write_index + 1;
  if (next_write_index >= global_console.uart_info.tx_buf_len)
  {
    next_write_index = 0;
  }

  if (next_write_index == global_console.uart_info.tx_buf_read_index) 
  {
    INC_SATURATE_U16(global_console.uart_info.stats.tx_buf_overrun_err);
    CRITICAL_END();
    return;
  }

  global_console.uart_info.tx_buf[global_console.uart_info.tx_buf_write_index] = (u8)ch;
  global_console.uart_info.tx_buf_write_index = next_write_index;

  // Ensure the TXE interrupt is enabled.
  __HAL_UART_ENABLE_IT(&global_console.uart_info.handle, UART_IT_TXE);

  CRITICAL_END();
}

INTERNAL char
console_read_ch(void)
{
  char result = 0;

  CRITICAL_BEGIN();

  if (global_console.uart_info.rx_buf_read_index == global_console.uart_info.rx_buf_write_index)
  {
    CRITICAL_END();
    return result;
  }

  u32 next_read_index = global_console.uart_info.rx_buf_read_index + 1;
  if (next_read_index >= global_console.uart_info.rx_buf_len)
  {
    next_read_index = 0;
  }

  result = (char)global_console.uart_info.rx_buf[global_console.uart_info.rx_buf_read_index];
  global_console.uart_info.rx_buf_read_index = next_read_index;

  CRITICAL_END();

  return result;
}

INTERNAL void 
console_printf(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  TempMemArena arena = temp_mem_arena_get(NULL, 0);
  
  String8 message = s8_fmt_nested(arena.arena, fmt, args);
  
  // TODO(Ryan): Add max print limit and indicate if reached by appending [!]
  for (u32 i = 0; i < message.size; i += 1)
  {
    console_write_ch((char)message.str[i]); 
  }

  temp_mem_arena_release(arena);

  va_end(args);
}

INTERNAL void 
console_printf_nested(char *fmt, va_list args)
{
  TempMemArena arena = temp_mem_arena_get(NULL, 0);
  
  String8 message = s8_fmt_nested(arena.arena, fmt, args);
  
  // TODO(Ryan): Add max print limit and indicate if reached by appending [!]
  for (u32 i = 0; i < message.size; i += 1)
  {
    console_write_ch((char)message.str[i]); 
  }

}

INTERNAL void
console_log(char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  u32 ms = HAL_GetTick();

  console_printf("%lu.%03lu ", ms / 1000U, ms % 1000U);
  console_printf_nested(fmt, args);

  va_end(args);
}

// uart ?, uart help
// uart status (show clients and their rx,tx index and buffer information)
// uart log (get log level), uart log off, uart log info
// uart stats, uart stats clear
// uart test

INTERNAL void
console_execute_cmd(String8 raw_str)
{
  TempMemArena arena = temp_mem_arena_get(NULL, 0);

  String8 space_split = s8_lit(" ");
  String8List cmd_tokens = s8_split(arena.arena, raw_str, &space_split, 1);

  String8Node *first_token = cmd_tokens.first;
  // NOTE(Ryan): Handle overarching commands
  if (cmd_tokens.node_count == 1)
  {
    if (first_token->string.str[0] == '?')
    {
      console_printf("CONSOLE:\n");
      console_printf("? (Display this information)\n");
      console_printf("+ (Enable logging)\n");
      console_printf("- (Disable logging)\n");
      console_printf("Change log level by entering: %s\n", LOG_LEVEL_NAMES_STR);
      console_printf("* (Display console info)\n");
      console_printf("test (Display test message)\n");

      console_printf("\nSUBSYSTEMS:\n");
      for (ConsoleCmdSystem *console_cmd_system = global_console.first;
          console_cmd_system != NULL;
          console_cmd_system = console_cmd_system->next)
      {
        console_printf("%.*s (", s8_varg(console_cmd_system->name));

        for (ConsoleCmd *console_cmd = console_cmd_system->first;
            console_cmd != NULL;
            console_cmd = console_cmd->next)
        {
          console_printf("%.*s%s", s8_varg(console_cmd->name), (console_cmd == console_cmd_system->last) ? "" : ",");
        }

        console_printf(")\n");
      }

    }
    else if (first_token->string.str[0] == '+')
    {
      global_console.log_active = true;
      console_printf("Logging enabled\n");
    }
    else if (first_token->string.str[0] == '-')
    {
      global_console.log_active = false;
      console_printf("Logging disabled\n");
    }
    else if (first_token->string.str[0] == '*')
    {
      char *log_level = log_level_str(global_console.log_level); 
      console_printf("%s logging is %s\n", log_level, global_console.log_active ? "enabled" : "disabled");
      console_printf("RX Buffer Overrun Error: %d\n", global_console.uart_info.stats.rx_buf_overrun_err);
      console_printf("TX Buffer Overrun Error: %d\n", global_console.uart_info.stats.tx_buf_overrun_err);
      console_printf("RX Overrun Error: %d\n", global_console.uart_info.stats.rx_overrun_err);
      console_printf("RX Noise Error: %d\n", global_console.uart_info.stats.rx_noise_err);
      console_printf("RX Frame Error: %d\n", global_console.uart_info.stats.rx_frame_err);
      console_printf("RX Parity Error: %d\n", global_console.uart_info.stats.rx_parity_err);
    } 
    else if (s8_match(first_token->string, s8_lit("test"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      console_printf("Test\n");
    }
    else if (s8_match(first_token->string, s8_lit("error"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_ERROR; 
      console_printf("Log level set to ERROR\n");
    }
    else if (s8_match(first_token->string, s8_lit("warning"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_WARNING; 
      console_printf("Log level set to WARNING\n");
    }
    else if (s8_match(first_token->string, s8_lit("info"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_INFO; 
      console_printf("Log level set to INFO\n");
    }
    else if (s8_match(first_token->string, s8_lit("debug"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_DEBUG; 
      console_printf("Log level set to DEBUG\n");
    }
    else if (s8_match(first_token->string, s8_lit("verbose"), S8_MATCH_FLAG_CASE_INSENSITIVE))
    {
      global_console.log_level = LOG_LEVEL_VERBOSE;
      console_printf("Log level set to VERBOSE\n");
    }
    else
    {
      console_printf("Unknown command: %.*s\n", s8_varg(first_token->string));
    }
  }
  else
  {
    String8Node *second_token = first_token->next;
    String8Node *third_token = second_token->next;

    for (ConsoleCmdSystem *console_cmd_system = global_console.first;
         console_cmd_system != NULL;
         console_cmd_system = console_cmd_system->next)
    {
      if (s8_match(first_token->string, console_cmd_system->name, S8_MATCH_FLAG_CASE_INSENSITIVE))
      {
        if (second_token->string.str[0] == '?')
        {
          for (ConsoleCmd *console_cmd = console_cmd_system->first;
              console_cmd != NULL;
              console_cmd = console_cmd->next)
          {
            console_printf("%.*s %.*s: %.*s\n", s8_varg(console_cmd_system->name), s8_varg(console_cmd->name), s8_varg(console_cmd->help));
          }

          return;
        }

        for (ConsoleCmd *console_cmd = console_cmd_system->first;
             console_cmd != NULL;
             console_cmd = console_cmd->next)
        {
          if (s8_match(second_token->string, console_cmd->name, S8_MATCH_FLAG_CASE_INSENSITIVE))
          {
            if (console_cmd->func(third_token) == CONSOLE_CMD_STATUS_FAILED)
            {
              console_printf("%s\n", console_cmd->help);
            }

            return;
          }
        }
      }
    }

    console_printf("Unknown command invocation: %.*s\n", s8_varg(raw_str));

  }

  temp_mem_arena_release(arena);
}
