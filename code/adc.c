// SPDX-License-Identifier: zlib-acknowledgement

// IMPORTANT(Ryan): When doing a makefile build, warnings won't be reissued if compiled. So, clean useful if debugging

// how are ADC channels distinct?
// channels to pins (VREFINT channel is constant reference voltage for ADC, VBAT channel is voltage for backup power domain; may also have a CPU temperature channel; would have to explicitly enable them as well)
void
init_adc(void)
{
  // TODO(Ryan): Don't have to enable GPIO if no external facing pins?  
  __HAL_RCC_TIM2_CLK_ENABLE();

  TIM_HandleTypeDef handle = ZERO_STRUCT;
  handle.Instance = TIM2;
  // IMPORTANT(Ryan): At first, just pick a generic value
  // Later, can do calculations to get more discerning values
  handle.Init.Prescaler = 5000;
  handle.Init.Period = 5000; 
  handle.Init.RepetitionCounter = 0x0;
  handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_Base_Init();

  // IMPORTANT(Ryan): Not all timers support triggers
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
  // number of channels 
  adc_handle.Init.NbrofConversion = 1;
  adc_handle.Init.DMAContinousRequests = ENABLE;
  adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  HAL_ADC_Init(&adc_handle);

  gpio_init.Mode = GPIO_MODE_ANALOG; 
  gpio_init.Pin = CHANNEL_PIN;
  // as many of these as channels
  ADC_ChannelConfTypeDef channel_init = ZERO_STRUCT;
  channel_init.Channel = ADC_CHANNEL_0; // pin specific
  // IMPORTANT(Ryan): If read channels one-at-a-time, then multiplexor used
  // for multiple channels, order in which sequenced
  channel_init.Rank = 1; 
  // because of how handling data in this case?
  channel_init.Offset = 0; 
  // IMPORTANT(Ryan): Faster sampling (i.e. low cycle number), less accurate
  // However, for high frequency signal like audio, require faster sampling rate
  channel_init.SamplingTime = ADC_SAMPLETIME_480CYCLES; 
  HAL_ADC_ConfigChannel(&channel_init, &adc_handle);

  // TODO(Ryan): Have easy way of viewing all interrupt priorities in use
  HAL_NVIC_SetPriority(ADC_IRQn, 0x8, 0x0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);

  HAL_ADC_Start_IT(&adc_handle);
  __HAL_TIM_ENABLE(&handle);

}

GLOBAL u16 global_adc_data[NUM_ADC_CHANNELS];
GLOBAL global_adc_channel_index;

void
ADC_IRQHandler(void)
{
  // end-of-conversion flag
  if (__HAL_ADC_GET_FLAG(&handle, ADC_FLAG_EOC))
  {
    global_adc_data[global_adc_channel_index++] = handle.Instance->DR; 
    if (global_adc_channel_index >= NUM_ADC_CHANNELS)
    {
      global_adc_channel_index = 0;
    }

    __HAL_ADC_CLEAR_FLAG(&handle, ADC_FLAG_EOC);
  }
}

GLOBAL u16 global_channel_data[2];

void
dma(void)
{
  // DMA has channels. A stream is where data comes from, i.e. channel will have a stream
  RCC_DMA_Clock();

  DMA_InitTypeDef dma_init = ZERO_STRUCT;
  dma_init.DMA_Channel = 0;
  // where getting data from
  dma_init.DMA_PeripheralBaseAddr = &ADC1->DR;
  dma_init.DMA_MemoryBaseAddr = global_channel_data;
  dma_init.DMA_Dir = DMA_DIR_PeripheralToMemory;
  // number of items to copy
  dma_init.DMA_BufferSize = 2;

  dma_init.DMA_PeripheralInc = DMA_PeriphalIncDisable;
  dma_init.DMA_MemoryInc = DMA_MemoryIncEnable;

  dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

  // continously perform copy (this could be optimised)
  dma_init.DMA_Mode = DMA_Mode_Circular;

  dma_init.DMA_Priority = DMA_Priority_Medium;

  // ??
  dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;

  // ??
  dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

  DMA_Init(&dma_init, DMA1_STREAM_0);
  DMA_Start(DMA1_STEAM_0);

  // IMPORTANT(Ryan): Inside of ADC init
  // ADC_DMAEnableRequest();
  // ADC_DMARequestAfterLastTransferCmd(); (start request after conversion finished)
  // So, seems no interrupt here?

}
