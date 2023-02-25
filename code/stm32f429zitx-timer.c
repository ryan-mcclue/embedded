// SPDX-License-Identifier: zlib-acknowledgement

#include "stm32f429zitx-timer.h"

GLOBAL Timers global_timers;

INTERNAL void
stm32f429zitx_create_timers(MemArena *perm_arena, u32 max_num_timers)
{
  global_timers.timer_info = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, TimerInfo, max_num_timers);
  global_timers.max_num_timers = max_num_timers;
  LOG_DEBUG("Timers created\n");
}

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
      timer_info->state = TIMER_STATE_RUNNING;
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
  if (timer_id > 0 && timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    ASSERT(timer_info->state != TIMER_STATE_UNUSED);

    timer_info->state = TIMER_STATE_STOPPED;
  }
}

INTERNAL void
timer_start(u32 timer_id)
{
  if (timer_id > 0 && timer_id < global_timers.max_num_timers)
  {
    TimerInfo *timer_info = &global_timers.timer_info[timer_id];

    ASSERT(timer_info->state != TIMER_STATE_UNUSED);

    timer_info->start_time = HAL_GetTick();
    timer_info->state = TIMER_STATE_RUNNING;
  }
}

INTERNAL void
timer_release(u32 timer_id)
{
  if (timer_id > 0 && timer_id < global_timers.max_num_timers)
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
