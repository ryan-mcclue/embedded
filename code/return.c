// SPDX-License-Identifier: zlib-acknowledgement


#if 0
// TODO: perhaps debug log include __LINE__ and __func__?

//  void __io_putchar(char c)
//  {
//      HAL_UART_Transmit(&CONFIG_CONSOLE_UART, (uint8_t*)&c, 1, HAL_MAX_DELAY);
//      // Line conditioning to automatically do a CR after a NL.
//      if (c == '\n') __io_putchar('\r');
//  }

typedef struct SerialInit SerialInit;
struct SerialInit
{
  rx/tx fifo, irq priority, pins
};

typedef int32_t (*cmd_func)(int32_t argc, const char** argv);

struct cmd_cmd_info {
    const char* const name; // Name of command
    const cmd_func func;    // Command function
    const char* const help; // Command help string
};

typedef struct ConsoleInfo ConsoleInfo
{
    const char* const name;          // Client name (first command line token)
    const int32_t num_cmds;          // Number of commands.
    const struct cmd_cmd_info* const cmds; // Pointer to array of command info
    int32_t* const log_level_ptr;    // Pointer to log level variable (or NULL)
    const int32_t num_u16_pms;       // Number of pm values.
    uint16_t* const u16_pms;         // Pointer to array of pm values
    const char* const* const u16_pm_names; // Pointer to array of pm names
};

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


INTERNAL void
stm32f069_console_init(SerialInit serial_init, )
{
  b32 send_cr_after_nl; 
  // devices would register as clients to main console?
  // in fact, last client is 'main' with its own set of commands

// Names of performance measurements.
static const char* cnts_u16_names[NUM_U16_PMS] = {
    "uart rx overrun err",
    "uart rx noise err",
    "uart rx frame err",
    "uart rx parity err",
    "tx buf overrun err",
    "rx buf overrun err",
};

// Data structure with console command info.
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_ttys_status,
        .help = "Get module status, usage: ttys status",
    },
    {
        .name = "test",
        .func = cmd_ttys_test,
        .help = "Run test, usage: ttys test [<op> [<arg>]] (enter no op/arg for help)",
    }
};

// Data structure passed to cmd module for console interaction.
static struct cmd_client_info cmd_info = {
    .name = "ttys",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
    .num_u16_pms = NUM_U16_PMS,
    .u16_pms = cnts_u16,
    .u16_pm_names = cnts_u16_names,
};

}

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
  // the 'performance' aspect of each modules is just a series of counters for various things
  // so, > uart pm

  //log_info("Initialising console\n");

  // timer

  // system commands:
  //   version, reset (return 'Starting'), help, status
        // NVIC_SystemReset();
  
  // all commands should return text like OK or 1 to allow for testing

  // create UART init, then move onto console etc.
  // RETURN_CODE return_code = stm32f069_console_init();
  // if (return_code != RETURN_OK)
  // {
  //   log_error("Console init error: %d\n", return_code);
  //   // saturate means clamp within bounds
  //   INC_U16_SATURATE(global_error_count[INIT_ERRORS]);
  // }

  // stm32f069_heartbeat_led_init();

  // log_info("Entering superloop\n");

  // while (true)
  // {
  //   console_execute();

  //   temperature_sensor_execute();
  // }
}
