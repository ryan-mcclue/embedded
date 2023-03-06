// SPDX-License-Identifier: zlib-acknowledgement

  // high-res 16/32bit timer very flexible:
  //   * irq
  //   * rotary encoder?
  //   * pwm
  //   * adc (effectively set sample rate)
  // timer properties: 
  //   * clock speed (based on bus its on)
  //   * num channels (so for PWM, want more?)
  //   * prescaler, period?
  
// have: timer period, repeating, repeat count
typedef struct HighResTimer HighResTimer;
struct HighResTimer
{
  u32 clock_speed;
  u32 num_channels;
  u32 period; // the same as counter target?
  u32 prescaler; // how much clock source is scaled, so 100MHz prescaler 4 gives 25MHz
  u32 counter_target, counter_value;
  u32 irq_priority;
};

INTERNAL void
init_high_res_timer1(void)
{
  HighResTimer timer1 = ZERO_STRUCT;
  // IMPORTANT(Ryan): Might have to take into account clock doubling, e.g. multiply by 2
  timer1.clock_speed = HAL_RCC_GetPCLK2Frequency();
  timer1.num_channels = 4;
  timer1.size = 16_BIT;
  timer1.base = TIM1;
  timer1.irq = TIM1_UP_TIM10_IRQn;

  __HAL_RCC_TIM1_CLK_ENABLE();
}

void
interrupt_handler(void)
{
  // checking important for shared interrupt
  if (__HAL_TIM_GET_FLAG(&handle, TIM_FLAG_UPDATE))
  {
    switch (mode)
    {
      case TIMER_CONTINUOUS:
      {
       // do_nothing();
      } break;
      case TIMER_MULTIPLE:
      {
        if (counter_value++ >= counter_target)
        {
          stop();
        }
      } break;
      case TIMER_SINGLE:
      {
        stop();
      } break;
    }
  }
}
