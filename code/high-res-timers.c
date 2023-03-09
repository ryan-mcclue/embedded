// SPDX-License-Identifier: zlib-acknowledgement

// TODO(Ryan): PID control loops

// TODO(Ryan): With debugging, also inspect peripheral registers
// Could just use an IO line if LED/serial debugging to impactful

  // high-res 16/32bit timer very flexible:
  //   * irq
  //   * rotary encoder:
  //   (uses quadrature signal of encoder to driver counter, rather than bus clock)
  //   (sends two signals? could just bit-bang with GPIO?)
  //   (clockwise: clk high first, anti: data high first) 
  //   (want timer counter register to be triggered by quadrature signal, rather than clock)
  //   * pwm
  //   * adc (effectively set sample rate)
  //   * dma (becomes more important when CPU computing UI)
  // timer properties: 
  //   * clock speed (based on bus its on)
  //   * num channels (so for PWM, want more?)
  //   * prescaler, period?
  
// have: timer period, repeating, repeat count
typedef struct HighResTimer HighResTimer;
struct HighResTimer
{
  u32 clock_speed;
  u32 num_channels; // more relevent for PWM with channel specific output compare register
  u32 prescaler; // the size of this dependent of timer size (check the size of this as could overflow)
  // how much clock source is scaled 
  // so 100MHz clock, prescaler 4, gives 25MHz, i.e. 25billion counter ticks per second
  u32 period; 
  // how many ticks till interrupt triggered 
  u32 irq_priority;
  u32 counter_target, counter_value;
  callback_func cb;
};


INTERNAL void
init_high_res_timer1_irq(void)
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

void
timer_quadrature_signal(void)
{
  GPIO_PIN_4 | GPIO_PIN_5, AF2_TIM3
  
  TIM_HandleTypeDef handle = ZERO_STRUCT;
  handle.Instance = TIM3;
  handle.Init.Prescaler = 0;
  handle.Init.Period = 1024; // somewhat arbitrary number
  handle.Init.RepetitionCounter = 0xFF;
  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  TIM_Encoder_InitTypeDef encoder = ZERO_STRUCT;
  encoder.EncoderMode = TIM_ENCODER_TI12; // ???
  encoder.IC1Polarity = TIM_ICPOLARITY_RISING;
  encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  encoder.IC1Prescaler = TIM_ICPSC_DIV1;
  encoder.IC1Filter = 0; // no need for filter as minimal noise?
  encoder.IC2Polarity = TIM_ICPOLARITY_RISING;
  encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  encoder.IC2Prescaler = TIM_ICPSC_DIV1;
  encoder.IC2Filter = 0;
  HAL_TIM_Encoder_Init(&handle, &encoder);

  HAL_TIM_Encoder_Start(&encoder, TIM_CHANNEL_ALL);

  // in superloop
  // IMPORTANT(Ryan): Must store prev and cur values to determine if wraparound has occured
  // probably also store signed value, i.e. negative
  u16 encoder_counter_val = __HAL_TIM_GET_COUNTER(&encoder); // __HAL_TIM_SET_COUNTER(&encoder, 0);
  // increases if clockwise, decreases if counterclockwise
  // the amount changed is determined by pulse count, i.e. might be 4
  // by comparing this with last value, could calculate accleration
  
  // IMPORTANT(Ryan): Start with ms signal triggering and work way down
  // TODO(Ryan): Add a delay if wanting to see signal change easier, e.g. button press
  // TODO(Ryan): Use peripheral decoders in oscilloscope to see bugs easier in a protocol
  // TODO(Ryan): How to measure signal frequency on oscilloscope. cursors? 
}

// TODO(Ryan): How to know when to prevent nested interrupts in an ISR?

// IMPORTANT(Ryan): Timer channels share counter register, period and prescaler.
void
pwm_init(u32 channel_select)
{
  gpio_init.Alternate = GPIO_AF2_TIM4;

  TIM_HandleTypeDef handle = ZERO_STRUCT;
  handle.Instance = TIM4;
  handle.Init.Prescaler = 100;
  handle.Init.Period = 256; // this determines granularity, e.g. 256 brightness levels? 
  // however, must make sure are within certain frequency, e.g. servo motor requirements
  handle.Init.RepetitionCounter = 0xFF; // ??
  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  // various timer methods change counting mechanics?
  HAL_TIM_PWM_Init(&handle);

  TIM_OC_InitTypeDef channel_init = ZERO_STRUCT;
  channel_init.OCMode = TIM_OCMODE_PWM1;
  channel_init.OCIdleState = TIM_OCIDLESTATE_SET;
  channel_init.Pulse = 0;
  channel_init.OCPolarity = TIM_OCPOLARITY_HIGH;
  channel_init.OCFastMode = TIM_OCFAST_ENABLE;
  HAL_TIM_PWM_ConfigChannel(&handle, &channel_init, TIM_CHANNEL_1);

  HAL_TIM_PWM_Start(&handle, TIM_CHANNEL_1);

  // IMPORTANT(Ryan): This controls the duty cycle
  // capture compare register
  // 128 is half of signal period of 256; so half on?
  __HAL_TIM_SET_COMPARE(&handle, TIM_CHANNEL_1, 128);
  // could be used to control brightness of LED
}

void
rgb_led(void)
{
  // using a common anode
  u8 r, g, b; 
  u8 brightness;

  b32 inverted; // true: common-cathode, false: common-anode

  TIM_OC_InitTypeDef red_channel = ZERO_STRUCT;
}

void
rgb_led_set_colour(u32 colour)
{
  f32 brightness_ratio = (brightness / period);  
  f32 r = red * brightness_ratio;

  set_pwm_val(red_channel, invert ? period - r : r);
}

// if components draw a lot of current, don't want to power from board as could fry onboard regulator?
// (in this limitation to safeguard sensitive electronics on board?)
// servo motors draw more current if actuating more load, i.e. high torque?

// IMPORTANT(Ryan): Higher current PSU is fine, the voltage is what needs to be the same
// Current draw determined by load, a.k.a resistance. So, will only draw what is needed

// Also, always use switching regulators for high power applications, because unlike linear regulator, switching regulators do not dissipate much heat and are highly efficient at high loads.

// regulator and converter same thing (prefer over Zener diode)
//   * load regulation (how much current draw affects output voltage)
//   * line regulation (how much input voltage affects output voltage)
//   * drop-out voltage (at least how much voltage above the output voltage the input voltage should be to not cause problems) 
//   * buck (step-down)
//   * boost (step-up)
//   * buck-boost
// linear regulator essentially dumps excess voltage
// switching has a PWM signal that essentially turns on and off; power-efficient
// look at efficiency curve to see optimal conditions


// IMPORTANT(Ryan): Always know base frequency of timers, e.g. 100MHz
// 100MHz <-> 50Hz
//  1. 100MHz/50Hz = 2MHz
//  2. Decide on 20000 steps/granularity as equates nicely to 20ms period (so take into account period here, e.g. 1500 == 1.5ms)
//  3. 2MHz/20000 = period

// servos utilise PID algorithm with potentiometer feedback.
// it looks at input signal and compares that with potetiometer to see if it needs to move or not
// 50Hz common
// to run slower, perhaps only update at 10ms
void
servos(void)
{
  // IMPORTANT(Ryan): These are with respect to OC counter values
  u32 centre_pos = 1500;
  min_pos = 1000;
  max_pos = 2000;
  current_pos = centre_pos;
}

void
set_servo_pos(f32 pos)
{
  if (pos == 0.0f)
  {
    current_pos = centre_pos;
  }
  else if (pos > 0.0f)
  {
    // map pos to range: centre_pos <-> max_pos
    set_pwm_val(servo_channel, current_pos);
  }
  else
  {

  }
}
