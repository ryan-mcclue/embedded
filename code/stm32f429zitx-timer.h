// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(STM32F429ZITX_TIMER_H)
#define STM32F429ZITX_TIMER_H

typedef u32 TIMER_STATE;
enum TIMER_STATE 
{
  TIMER_STATE_UNUSED = 0,
  TIMER_STATE_STOPPED,
  TIMER_STATE_RUNNING,
  TIMER_STATE_EXPIRED,
};

typedef void (*timer_cb)(u32 timer_id, void *user_data);

typedef struct TimerInfo TimerInfo;
struct TimerInfo 
{
  TIMER_STATE state;
  u32 period_ms;
  u32 start_time;
  timer_cb cb;
  void *cb_user_data;
  b32 want_restart;
};

typedef struct Timers Timers;
struct Timers
{
  TimerInfo *timer_info;
  u32 max_num_timers;
};


#endif
