// SPDX-License-Identifier: zlib-acknowledgement


#if 0
// TODO: perhaps debug log include __LINE__ and __func__?

//  void __io_putchar(char c)
//  {
//      HAL_UART_Transmit(&CONFIG_CONSOLE_UART, (uint8_t*)&c, 1, HAL_MAX_DELAY);
//      // Line conditioning to automatically do a CR after a NL.
//      if (c == '\n') __io_putchar('\r');
//  }

struct cmd_arg_val {
    char type;
    union {
        void*       p;
        uint8_t*    p8;
        uint16_t*   p16;
        uint32_t*   p32;
        int32_t     i;
        uint32_t    u;
        const char* s;
    } val;
};


// Names of performance measurements.
static const char* cnts_u16_names[NUM_U16_PMS] = {
    "uart rx overrun err",
    "uart rx noise err",
    "uart rx frame err",
    "uart rx parity err",
    "tx buf overrun err",
    "rx buf overrun err",
};


void
console_run(MemArena *arena)
{
  if (first_run) log_plain(PROMPT_CHAR);
  while (serial_data_available(console))
  {
    // parse command into tokens
    // first check for wildcards like '*/?/logging-off'
    // then parse on name, so 'uart' 'command_name'

    console_execute();
    log_plain(PROMPT_CHAR);
  }
}

#endif

GLOBAL permanent_arena, temp;

void
main(void)
{
  //log_info("Initialising console\n");

  // timer

  // system commands:
  //   version, reset (return 'Starting'), help, status
        // NVIC_SystemReset();
  

  // if (return_code != RETURN_OK)
  // {
  //   log_error("Console init error: %d\n", return_code);
  //   // saturate means clamp within bounds
  //   INC_U16_SATURATE(global_error_count[INIT_ERRORS]);
  // }

  // stm32f069_heartbeat_led_init();

  // log_info("Entering superloop\n");
}
