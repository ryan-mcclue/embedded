// SPDX-License-Identifier: zlib-acknowledgement

// without DAC, require PWM and an output filter?
// typically channels can be used simultaneously, i.e. dual mode; not a multiplexor?

// impedence more important for analog as like AC?
// analog mode want high input impe, low output impe so circuitry not disturbing analog values

void
dac_init(void)
{
  gpio_init.Mode = ANALOG;
  
  DAC_InitTypeDef dac_init = ZERO_STRUCT;
  dac_init.DAC_Trigger = DAC_Trigger_None;
  // we will do our own
  dac_init.DAC_WaveGeneration = None;
  dac_init.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1);
  DAC_Start();

  // write into DAC data register
}
