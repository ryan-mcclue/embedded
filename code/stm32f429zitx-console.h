// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(STM32F429ZITX_CONSOLE_H)
#define STM32F429ZITX_CONSOLE_H

typedef u32 LOG_LEVEL;
enum {
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_VERBOSE,
  LOG_LEVEL_COUNT
};

#define LOG_LEVEL_NAMES_STR "(error|warning|info|debug|verbose)"

INTERNAL char *
log_level_str(LOG_LEVEL log_level)
{
  char *result = "unknown";

  switch (log_level)
  {
    default: break;
    case LOG_LEVEL_ERROR:
    {
      result = "ERROR"; 
    } break;
    case LOG_LEVEL_WARNING:
    {
      result = "WARNING"; 
    } break;
    case LOG_LEVEL_INFO:
    {
      result = "INFO"; 
    } break;
    case LOG_LEVEL_DEBUG:
    {
      result = "DEBUG"; 
    } break;
    case LOG_LEVEL_VERBOSE:
    {
      result = "VERBOSE"; 
    } break;
  }

  return result;
}

#define LOG_ERROR(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_ERROR) \
      console_log("ERROR (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_WARNING(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_WARNING) \
      console_log("WARNING (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_INFO(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_INFO) \
      console_log("INFO (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_DEBUG(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_DEBUG) \
      console_log("DEBUG (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
  } while (0) 

#define LOG_VERBOSE(SUBSYSTEM, fmt, ...) \
  do { \
    if (global_console.log_active && global_console.log_level >= LOG_LEVEL_VERBOSE) \
      console_log("VERBOSE (" STRINGIFY(SUBSYSTEM) "): " fmt, ##__VA_ARGS__); \
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
  u32 rx_buf_len, tx_buf_len;
  USART_TypeDef *uart_base;

  // TODO(Ryan): Consider specifying interrupt priority
};

typedef u32 CONSOLE_CMD_STATUS;
enum {
  CONSOLE_CMD_STATUS_FAILED,
  CONSOLE_CMD_STATUS_SUCCEEDED,
};

typedef CONSOLE_CMD_STATUS (*console_cmd_func)(String8Node *node);

typedef struct ConsoleCmd ConsoleCmd;
struct ConsoleCmd
{
  ConsoleCmd *next;
  String8 name, help; // uart status: <help> (so help should incorporate a description)

  console_cmd_func func;
};

typedef struct ConsoleCmdSystem ConsoleCmdSystem;
struct ConsoleCmdSystem
{
  ConsoleCmdSystem *next;
  String8 name; 

  ConsoleCmd *first;
  ConsoleCmd *last;
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

typedef struct UartInfo UartInfo;
struct UartInfo
{
  UART_HandleTypeDef handle;

  u32 rx_buf_read_index, rx_buf_write_index;
  u8 *rx_buf;
  u32 rx_buf_len;
  
  u32 tx_buf_read_index, tx_buf_write_index;
  u8 *tx_buf;
  u32 tx_buf_len;
  
  UartStats stats;
};

typedef struct Console Console;
struct Console
{
  b32 log_active;
  LOG_LEVEL log_level;

  ConsoleCmdSystem *first;
  ConsoleCmdSystem *last;

  MemArena *perm_arena;

  u32 cmd_str_buf_len, cmd_str_buf_cursor;
  String8 cmd_str;

  UartInfo uart_info;
};

#endif
