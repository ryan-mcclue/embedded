// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): PID control loops

// TODO(Ryan): With debugging, also inspect peripheral registers
// Could just use an IO line if LED/serial debugging to impactful

  // high-res 16/32bit timer very flexible:
  //   * irq
  //   * rotary encoder? 
  //   (uses quadrature signal of encoder to driver counter, rather than bus clock)
  //   (sends two signals? could just bit-bang with GPIO?)
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
  u32 prescaler; // the size of this dependent of timer size (check the size of this as could overflow)
  // how much clock source is scaled 
  // so 100MHz clock, prescaler 4, gives 25MHz, i.e. 25billion counter ticks per second
  u32 period; 
  // how many ticks till interrupt triggered 
  u32 irq_priority;
  u32 counter_target, counter_value;
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

  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.RepetitionCounter = 0x0;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // auto reload register
  HAL_TIM_Base_Init();

  __HAL_RCC_TIM1_CLK_ENABLE();

  NVIC_EnableIRQ();
}

void
start_timer(void)
{
  __HAL_TIM_ENABLE_IT(&handle, TIM_IT_UPDATE);
  __HAL_TIM_ENABLE(&handle);
}

void
stop_timer(void)
{
  __HAL_TIM_DISABLE(&handle);
  __HAL_TIM_DISABLE_IT(&handle, TIM_IT_UPDATE);
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

  __HAL_TIM_CLEAR_FLAG(&handle, TIM_FLAG_UPDATE);
}
