// SPDX-License-Identifier: zlib-acknowledgement
 
// PWM still 5V, albeit in pulses
// without DAC, require very high frequency PWM and output filters to smooth transitions

// typically channels can be used simultaneously, i.e. dual mode; not a multiplexor?

// impedence more important for analog as like AC?
// analog mode want high input impe, low output impe so circuitry not disturbing analog values

TIM6_DAC_IRQHandler(void)
{

}

void
dac_init(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();

  TIM_HandleTypeDef handle = ZERO_STRUCT;
  handle.Instance = TIM2;
  // TODO(Ryan): 16MHz clock
  handle.Init.Prescaler = 5000;
  handle.Init.Period = 5000; 
  handle.Init.RepetitionCounter = 0x0;
  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_Base_Init();

  TIM_MasterConfigTypeDef mc_init = ZERO_STRUCT;
  mc_init.MasterOutputTrigger = TIM_TRGO_UPDATE;
  HAL_TIMEx_MasterConfigSynchronization(&handle, &mc_init);

  GPIO_InitTypeDef gpio_init = ZERO_STRUCT;
  gpio_init.Pin = GPIO_PIN_4;
  gpio_init.Mode = GPIO_MODE_ANALOG;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  u32 dac_dio = dio_add_output(s8_lit("passive_buzzer"), &gpio_init, GPIOA, 0);
  // DAC 1/1, PA_4
  __HAL_RCC_DAC_CLK_ENABLE();

  HAL_TIM_IRQHandler
  DAC_HandleTypeDef dac_handle = ZERO_STRUCT;
  if (HAL_DAC_Init(&dac_handle) == HAL_OK)
  {
    DAC_ChannelConfTypeDef dac_channel = ZERO_STRUCT;
    // trigger takes it out of holding register, so still copy over in timer irq
    dac_channel.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
    dac_channel.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

    if (HAL_DAC_ConfigChannel(&dac_handle, &dac_channel, DAC_CHANNEL_ONE) == HAL_OK)
    {
      if (HAL_DAC_Start(&dac_handle, DAC_CHANNEL_ONE) == HAL_OK)
      {

      }
    }
  }

  HAL_NVIC_SetPriority(ADC_IRQn, 0x8, 0x0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);

  __HAL_TIM_ENABLE(&handle);

// Each time a DAC interface detects a rising edge on the selected trigger source, the last data stored into the DAC_DHRx register are transferred into the DAC_DORx register

	HAL_DAC_SetValue(&DAC_Handle[(uint8_t)DACx], DAC_CHANNEL_1, DAC_ALIGN_12B_R, value);

       3.3 * DOR / 4095
}
// with active speaker, just adjust period of PWM to get varying sound as just turning built-in oscillator on and off

// Higher addresses in flash typically require special priveleges to write to, e.g. trace control?

// TODO(Ryan): Difference between SWD and JTAG? SWD less pins arm specific? Doesn't offer trace pins like in JTAG?
// How is UART console used in field, i.e. how to get access to pins?
// To save UART pins for application, can use SWO (single-wire-output) (kind of like a single TX pin?)
// SWO is part of the ARM CoreSight Debug block which usually is part of Cortex-M3, M4 and M7
// Segger RTT is faster and doesn't require more pins. However, uses more RAM on target
// https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/
