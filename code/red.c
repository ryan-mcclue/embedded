  // often interrupts and exceptions used interchangeably
  // In ARM, an interrupt is a type of exception (changing normal flow of program)
  // Exception number (offset into vector table, i.e. exception handler), priority level,
  // synchronous/asynchronous, state (pending, active, inactive)
  // Index 0 of vector table is reset value of stack pointer (rest exception handlers) 
  // On Cortex-M, 6 exceptions always supported: reset, nmi, hardfault, SVCall, PendSV, SysTick 
  // External interrupts start from 16 and are configured via NVIC (specifically registers within it)
  // Will have to first enable device to generate the interrupt, then set NVIC accordingly
  // The startup file will define the interrupt handlers names to which we should define
  // Typically an NVIC interrupt handler will have 'sub interrupts' which we can determine with ITStatus()
  //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  //NVIC_EnableIRQ(USART3_IRQn); // cmsis m4

  while (1)
  {
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
    uint16_t adc_data = ADC_GetConversionValue(ADC1);
    float volts_read = (adc_data / ((1 << 12) - 1)) * 5.0f;
    /* temp sensor reading:
     * volts = (adc_data[1] / ((1 << 12) - 1)) * 3.3f;
     * (NOTE: ADCs typically have their own reference voltage to filter out noise)
     * volts -= 0.76; 
     * float celsius = volts / 0.0025f;
     * celsius += 25; (this may not have to be added?)
     */
  }

  // DMA has streams (where data comes in). Channels feed into streams
  // identify the channel, stream and DMA controller number for desired peripheral 
  // (found in reference manual)
  // identify where inside the peripheral we are actually reading data from (most likely a data register)
  // channel 0, stream 0, DMA2 
  

  
  // NOTE(Ryan): Create a 1 second pulse
  timer_init.TIM_Prescaler = 8399; // 84MHz, so 8400 - 1
  timer_init.TIM_Period = 9999; // divide by 10000, so 10000 - 1 
  // The period would affect the resolution for PWM, the prescaler the frequency of PWM
  
  // IMPORTANT(Ryan): For PWM, we require higher frequency, e.g. 1kHz so it appears silky smooth  
  // We relate this value to the period to obtain duty cycle.
  // This is half period, so a 50/50, 50% duty cycle

  // Say we want an output frequency of 1Hz, we play with prescaler and counter values that fit within the 16 bits provided in relation to the input clock
  // E.g. 42MHz in, divide by 42000 (in range of 16bit) gives 1000Hz. Then count to 1000
  // A formula for calculating this will often be given (update event period)
