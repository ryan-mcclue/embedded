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

  // write into DAC data register, e.g. max 4095 for 12bit
  // will correspond to voltage level, e.g. max 4095 3.3V
  
}

// Higher addresses in flash typically require special priveleges to write to, e.g. trace control?

// TODO(Ryan): Difference between SWD and JTAG? SWD less pins arm specific? Doesn't offer trace pins like in JTAG?
// How is UART console used in field, i.e. how to get access to pins?
// To save UART pins for application, can use SWO (single-wire-output) (kind of like a single TX pin?)
// SWO is part of the ARM CoreSight Debug block which usually is part of Cortex-M3, M4 and M7
// Segger RTT is faster and doesn't require more pins. However, uses more RAM on target
// https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/
