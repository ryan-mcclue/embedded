// SPDX-License-Identifier: zlib-acknowledgement

// gtu-u7 (gtu developed by u-blox)

// slow blink is intended to count to indicate a condition

typedef struct Blinky Blinky;
struct Blinky
{
  u32 dio_index;
  u32 num_blinks;
  u32 period_ms;
  BLINKY_STATE state;
  u32 timer_id;
};

INTERNAL void
blinky_init(void)
{
  blinky.timer_id = timer_create(0, blinky_callback, NULL);

  blink_start();
}

INTERNAL void
blinky_start(void)
{
  blinky.state = BLINKY_STATE_PRE_BLINK_DELAY; 
  timer_start(blinky.timer_id, PRE_BLINK_DELAY_MS);
}

INTERNAL void
blinky_callback(void)
{
  u32 next_timer_id = 0;

  switch (blinky.state)
  {

  }

  blinky.state = new_state;

  dio_set();

  timer_start();
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

typedef u32 BLINKY_STATE;
enum {
  BLINKY_STATE_OFF,
  BLINKY_STATE_SEPARATOR_ON,
  BLINKY_STATE_SEPARATOR_OFF,
  BLINKY_STATE_PRE_BLINK_DELAY,
  BLINKY_STATE_BLINK_ON,
  BLINKY_STATE_BLINK_OFF,
  BLINKY_STATE_POST_BLINK_DELAY,
};





