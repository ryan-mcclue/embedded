// SPDX-License-Identifier: zlib-acknowledgement

#include "blinky.h"

GLOBAL Blinkies global_blinkies;

INTERNAL void
blinkies_init(MemArena *perm_arena, u32 max_num_blinkies)
{
  LOG_DEBUG("Blinky init\n");

  global_blinkies.blinkies = MEM_ARENA_PUSH_ARRAY_ZERO(perm_arena, Blinky, max_num_blinkies);
  global_blinkies.max_num_blinkies = max_num_blinkies;
}

// various ways to implement state machine
// some use tables for many states
// for simple ones, use this timer

// state machine: (typically deterministic?)
//   * states (Moore produces output when remaining?). Always have fail state (even if implicit)
//   * transitions (Meley produces output)
//   * events

// seems that Moore and Meley definitions are too rigid, as sometimes (produce output/perform action) on a transition and sometimes won't

// FSMs are useful in times where you have to wait and don't have threads?
// FSMS good for reactive systems (of which embedded are)

// event-driven and table-driven FSMs?
// like autosar/cubeIDE embedded has a lot of 'model-driven' software development

INTERNAL void
blinky_callback(u32 timer_id, void *data)
{
  Blinky *blinky = (Blinky *)data; 

  BLINKY_STATE next_state = 0;
  b32 led_on = false;
  u32 next_timer_value = 0;

  switch (blinky->state)
  {
    default:
    {
      LOG_ERROR("Unknown blinky state\n");
    } break;
    case BLINKY_STATE_PRE_BLINK_DELAY:
    {
      next_state = BLINKY_STATE_BLINK_ON;
      next_timer_value = blinky->period_ms;
      led_on = true;
      blinky->blink_counter = 0;
    } break;
    case BLINKY_STATE_BLINK_ON:
    {
      if (++blinky->blink_counter < blinky->num_blinks) 
      {
        next_state = BLINKY_STATE_BLINK_OFF;
        led_on = false;
        next_timer_value = blinky->period_ms; 
      } 
      else 
      {
        next_state = BLINKY_STATE_POST_BLINK_DELAY;
        led_on = false;
        next_timer_value = blinky->post_blink_delay;
      }
    } break;
    case BLINKY_STATE_BLINK_OFF:
    {
      next_state = BLINKY_STATE_BLINK_ON;
      led_on = true;
      next_timer_value = blinky->period_ms;
    } break;
    case BLINKY_STATE_POST_BLINK_DELAY:
    {
      next_state = BLINKY_STATE_PRE_BLINK_DELAY;
      led_on = false;
      next_timer_value = blinky->pre_blink_delay;
    } break;
  }

  blinky->state = next_state;

  dio_output_set(blinky->dio_index, led_on);

  timer_set_period(blinky->timer_id, next_timer_value); 
  timer_start(blinky->timer_id);
}

INTERNAL u32
blinky_create(u32 dio_index, u32 num_blinks, u32 period_ms, u32 pre_blink_delay, u32 post_blink_delay)
{
  // TODO(Ryan): Have a sentinel index that when handled by other functions just ignores
  u32 result = 0;

  if (global_blinkies.count_blinkies >= global_blinkies.max_num_blinkies)
  {
    LOG_WARNING("Not adding new blinky as reached maximum number\n");
  }
  else
  {
    Blinky *blinky = &global_blinkies.blinkies[global_blinkies.count_blinkies];

    blinky->num_blinks = num_blinks;
    blinky->period_ms = period_ms;
    blinky->pre_blink_delay = pre_blink_delay;
    blinky->post_blink_delay = post_blink_delay;

    blinky->timer_id = timer_create(blinky->pre_blink_delay, false, blinky_callback, blinky);

    blinky->dio_index = dio_index;

    result = global_blinkies.count_blinkies++;
  }

  return result;
}

INTERNAL void
blinky_start(u32 blinky_id)
{
  if (blinky_id < global_blinkies.max_num_blinkies)
  {
    Blinky *blinky = &global_blinkies.blinkies[blinky_id];

    blinky->state = BLINKY_STATE_PRE_BLINK_DELAY; 
    timer_start(blinky->timer_id);
  }
}

