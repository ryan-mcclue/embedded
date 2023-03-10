// SPDX-License-Identifier: zlib-acknowledgement

#include "stm32f429zitx-timer.h"

GLOBAL Timers global_timers;


INTERNAL u32
timer_create(u32 period_ms, b32 want_restart, timer_cb cb, void  *cb_user_data)
{
  u32 result = MAX_U32;

  for (u32 timer_i = 0; timer_i < global_timers.max_num_timers; timer_i += 1)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_i];
    if (timer_info->state == TIMER_STATE_UNUSED)
    {
      timer_info->period_ms = period_ms;
      timer_info->start_time = HAL_GetTick();
      timer_info->state = TIMER_STATE_STOPPED;
      timer_info->cb = cb;
      timer_info->cb_user_data = cb_user_data;
      timer_info->want_restart = want_restart;

      result = timer_i;

      break;
    }
  }

  if (result == MAX_U32)
  {
    LOG_ERROR("Exceeded number of timers in use\n");
  }

  return result;
}

  // timers based on systick is 1ms; not spectacular resolution
  // important to recognise possible rollover when doing elapsed time calculations
  // high resolution timers more overhead, and in general these basic 'guard timers' don't require high resolution
  // furthermore, with higher res timers probably want callbacks to execute immediately in say an interrupt 

INTERNAL void
timer_stop(u32 timer_id)
{
  if (timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    ASSERT(timer_info->state != TIMER_STATE_UNUSED);

    timer_info->state = TIMER_STATE_STOPPED;
  }
}

INTERNAL void
timer_start(u32 timer_id)
{
  if (timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    ASSERT(timer_info->state != TIMER_STATE_UNUSED);

    timer_info->start_time = HAL_GetTick();
    timer_info->state = TIMER_STATE_RUNNING;
  }
}

INTERNAL void
timer_set_period(u32 timer_id, u32 period_ms)
{
  if (timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    timer_info->period_ms = period_ms;
  }
}

INTERNAL void
timer_release(u32 timer_id)
{
  if (timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    timer_info->state = TIMER_STATE_UNUSED;
  }

}

INTERNAL void
timers_update(void)
{
  LOCAL_PERSIST u32 previous_update_time_ms = 0;
  u32 now_ms = HAL_GetTick();

  // NOTE(Ryan): If superloop iterations running faster than 1ms, don't run as this will incorrectly run callback multiple times
  if (now_ms == previous_update_time_ms)
  {
    return;
  }

  previous_update_time_ms = now_ms;
  for (u32 timer_i = 0; timer_i < global_timers.max_num_timers; timer_i += 1) 
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_i];

    if (timer_info->state == TIMER_STATE_RUNNING) 
    {
      if ((now_ms - timer_info->start_time) >= timer_info->period_ms)
      {
        timer_info->state = TIMER_STATE_EXPIRED;
        timer_info->cb(timer_i, timer_info->cb_user_data);
        now_ms = HAL_GetTick();

        if (timer_info->want_restart)
        {
          timer_info->state = TIMER_STATE_RUNNING;
          timer_info->start_time += timer_info->period_ms;
        }
      }
    }
  }
}

INTERNAL char *
timer_state_str(TIMER_STATE state)
{
  char *result = "UNKNOWN";
  
  switch (state)
  {
    default: break;
    case TIMER_STATE_UNUSED:
    {
      result = "UNUSED";
    } break;
    case TIMER_STATE_STOPPED:
    {
      result = "STOPPED";
    } break;
    case TIMER_STATE_RUNNING:
    {
      result = "RUNNING";
    } break;
    case TIMER_STATE_EXPIRED:
    {
      result = "EXPIRED";
    } break;
  }
  
  return result;
}

INTERNAL CONSOLE_CMD_STATUS
timer_status_cmd(String8Node *remaining_args)
{
  CONSOLE_CMD_STATUS result = CONSOLE_CMD_STATUS_FAILED;

  u32 now_ms = HAL_GetTick();

  console_printf("Current millisecond timer=%lu\n", now_ms);

  console_printf("ID   Period   Start time Time left  Want Restart  State\n");
  console_printf("-- ---------- ---------- ---------- ------------- ------\n");
  for (u32 timer_i = 0; timer_i < global_timers.max_num_timers; timer_i += 1) 
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_i];
    if (timer_info->state == TIMER_STATE_UNUSED)
    {
      continue;
    }

    console_printf("%2lu %10lu %10lu %10lu %12s %s\n", 
                   timer_i, 
                   timer_info->period_ms,
                   timer_info->start_time,
                   (timer_info->state == TIMER_STATE_RUNNING) ?  (timer_info->period_ms - (now_ms - timer_info->start_time)) : 0,
                   timer_info->want_restart ? "Y" : "N",
                   timer_state_str(timer_info->state));
  }

  result = CONSOLE_CMD_STATUS_SUCCEEDED;

  return result;
}

INTERNAL void
timer_add_console_cmds(void)
{
  ConsoleCmdSystem *timer_system = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmdSystem);
  timer_system->name = s8_lit("timer");

  ConsoleCmd *status_cmd = MEM_ARENA_PUSH_STRUCT_ZERO(global_console.perm_arena, ConsoleCmd);
  status_cmd->name = s8_lit("status");
  status_cmd->help = s8_lit("Prints status");
  status_cmd->func = timer_status_cmd;

  SLL_QUEUE_PUSH(timer_system->first, timer_system->last, status_cmd);
  SLL_QUEUE_PUSH(global_console.first, global_console.last, timer_system);
}

INTERNAL void
stm32f429zitx_create_timers(MemArena *perm_arena, u32 max_num_timers)
{
  global_timers.timer_info = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, TimerInfo, max_num_timers);
  global_timers.max_num_timers = max_num_timers;
  LOG_DEBUG("Timers created\n");
  
  timer_add_console_cmds();
}

#if 0
static int32_t cmd_tmr_test(int32_t argc, const char** argv)
{
    struct cmd_arg_val arg_vals[2];
    uint32_t param1;
    uint32_t param2 = 0;
    int32_t rc;

    // Handle help case.
    if (argc == 2) {
        printf("Test operations and param(s) are as follows:\n"
               "  Get a non-callback tmr, usage: tmr test get <ms>\n"
               "  Get a callback tmr, usage: tmr test get_cb <ms> <cb-user-data>\n"
               "  Start a tmr, usage: tmr test start <tmr-id> <ms>\n"
               "  Release a tmr, usage: tmr test release <tmr-id>\n"
               "  Check if expired, usage: tmr test is_expired <tmr-id>\n");
        return 0;
    }

    if (argc < 4) {
        printf("Insufficent arguments\n");
        return MOD_ERR_BAD_CMD;
    }

    // Initial argument checking.
    if (strcasecmp(argv[2], "get_cb") == 0 ||
        strcasecmp(argv[2], "start") == 0) {
        if (cmd_parse_args(argc-3, argv+3, "uu", arg_vals) != 2)
            return MOD_ERR_BAD_CMD;
        param2 = arg_vals[1].val.u;
    } else {
        if (cmd_parse_args(argc-3, argv+3, "u", arg_vals) != 1)
            return MOD_ERR_BAD_CMD;
    }
    param1 = arg_vals[0].val.u;

    if (strcasecmp(argv[2], "get") == 0) {
        // command: tmr test get <ms>
        rc = tmr_inst_get(param1);
    } else if (strcasecmp(argv[2], "get_cb") == 0) {
        // command: tmr test get_cb <ms> <cb_user_data>
        rc = tmr_inst_get_cb(param1, test_cb_func, param2);
    } else if (strcasecmp(argv[2], "start") == 0) {
        // command: tmr test start <id> <ms>
        rc = tmr_inst_start(param1, param2);
    } else if (strcasecmp(argv[2], "release") == 0) {
        // command: tmr test release <id>
        rc = tmr_inst_release(param1);
    } else if (strcasecmp(argv[2], "is_expired") == 0) {
        // command: tmr test is_expired <id>
        rc = tmr_inst_is_expired(param1);
    } else {
        printf("Invalid operation '%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    printf("Operation returns %ld\n", rc);
    return 0;
}

/*
 * @brief Timer callback function for "tmr test" command.
 *
 * @param[in] tmr_id Timer ID.
 * @param[in] user_data User callback data.
 *
 * @return TMR_CB_RESTART or TMR_CB_NONE based on test logic.
 */
static enum tmr_cb_action test_cb_func(int32_t tmr_id, uint32_t user_data)
{
    log_debug("test_cb_func(tmr_id=%d user_data=%lu\n",
              tmr_id, user_data);
    return user_data == 0 ? TMR_CB_RESTART : TMR_CB_NONE;
}
#endif
