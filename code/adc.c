// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): When doing a makefile build, warnings won't be reissued if compiled. So, clean useful if debugging

// how are ADC channels distinct?
void
init_adc(void)
{
  gpio_init.Mode = GPIO_MODE_ANALOG; 

  TIM_HandleTypeDef handle = ZERO_STRUCT;
  handle.Instance = TIM2;
  // IMPORTANT(Ryan): At first, just pick a generic value
  // Later, can do calculations to get more discerning values
  handle.Init.Prescaler = 5000;
  handle.Init.Period = 5000; 
  handle.Init.RepetitionCounter = 0xFF;
  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  TIM_MasterConfigTypeDef mc_init = ZERO_STRUCT;
  // everytime gets to period value, trigger update
  mc_init.MasterOutputTrigger = TIM_TRGO_UPDATE;
  HAL_TIMEx_MasterConfigSynchronization(&handle, &mc_init);

  ADC_HandleTypeDef adc_handle = ZERO_STRUCT;
  adc_handle.Instance = ADC1;
  adc_handle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
  adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
  // if have multiple channels, want to be able to scan through them?
  adc_handle.Init.ScanConvMode = ENABLE;
  // timer is triggering each individual conversion
  adc_handle.Init.ContinousConvMode = DISABLE;
  adc_handle.Init.DiscontinousConvMode = DISABLE;
  adc_handle.Init.NbrOfDiscConversion = 0;
  adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  adc_handle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  // so bits are in right portion
  adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  // only want one channel at the moment
  adc_handle.Init.NbrofConversion = 1;
  adc_handle.Init.DMAContinousRequests = ENABLE;
  adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  HAL_ADC_Init(&adc_handle);

  ADC_ChannelConfTypeDef channel_init = ZERO_STRUCT;
  channel_init.Channel = ADC_CHANNEL_0; // pin specific
  // for multiple channels, order in which sequenced
  channel_init.Rank = 1; 
  // because of how handling data in this case?
  channel_init.Offset = 0; 
  // IMPORTANT(Ryan): Faster sampling (i.e. low cycle number), less accurate
  channel_init.SamplingTime = ADC_SAMPLETIME_480CYCLES; 
  HAL_ADC_ConfigChannel(&channel_init, &adc_handle);

  HAL_NVIC_SetPriority(ADC_IRQn, 0x8, 0x0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);

  HAL_ADC_Start_IT(&adc_handle);
  __HAL_TIM_ENABLE(&handle);

}

void
ADC_IRQHandler(void)
{
  if (__HAL_ADC_GET_FLAG(&handle, ADC_FLAG_EOC))
  {
    u16 data = handle.Instance->DR; 

    __HAL_ADC_CLEAR_FLAG(&handle, ADC_FLAG_EOC);
  }

}
