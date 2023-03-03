// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BLINKY_H)
#define BLINKY_H

typedef u32 BLINKY_STATE;
enum {
  BLINKY_STATE_OFF,
  BLINKY_STATE_PRE_BLINK_DELAY,
  BLINKY_STATE_BLINK_ON,
  BLINKY_STATE_BLINK_OFF,
  BLINKY_STATE_POST_BLINK_DELAY,
};

typedef struct Blinky Blinky;
struct Blinky
{
  u32 dio_index;

  u32 num_blinks;
  u32 blink_counter;

  u32 period_ms;
  u32 pre_blink_delay;
  u32 post_blink_delay;
  BLINKY_STATE state;

  u32 timer_id;
};

typedef struct Blinkies Blinkies;
struct Blinkies
{
  Blinky *blinkies;
  u32 max_num_blinkies;
  u32 count_blinkies;
};

#endif
