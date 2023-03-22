// SPDX-License-Identifier: zlib-acknowledgement

#define ARE_IN_IRQ() \
  (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)

#define SYSCLOCK_MHZ() \
  ((u16)(SystemCoreClock * (f32)(1e-6)))


#define INIT_CYCLE_COUNTER() \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk

#define ENABLE_CYCLE_COUNTER() \
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk

#define DISABLE_CYCLE_COUNTER() \
    (DWT->CTRL &= (~DWT_CTRL_CYCCNTENA_Msk))

#define GET_CYCLE_COUNTER() \
    (DWT->CYCCNT)

#define RESET_CYCLE_COUNTER() \
    (DWT->CYCCNT = (0))

INTERNAL void 
delay_us(u32 us)
{
  u64 start_cycles = GET_CYCLE_COUNTER();
  u64 cycles_per_us = (SystemCoreClock / 1000000);
  u64 cycles_to_delay = us * cycles_per_us;
  cycles_to_delay -= 12;
  while ((GET_CYCLE_COUNTER() - start_cycles) < cycles_to_delay);
}

INTERNAL void 
system_reset(void) 
{
	/* Call user callback function */
	
	NVIC_SystemReset();
}

typedef u32 CLOCK_SOURCE;
enum
{
	CLOCK_SOURCE_HSI,
	CLOCK_SOURCE_HSE,
	CLOCK_SOURCE_SYSCLK,
	CLOCK_SOURCE_PCLK1,
	CLOCK_SOURCE_PCLK2,
	CLOCK_SOURCE_HCLK
};

INTERNAL u32 
get_clock_speed(CLOCK_SOURCE clock_source) 
{
	u32 result = 0;

	switch (clock_source) 
  {
		default: break;
		case CLOCK_SOURCE_HSI:
    {
			result = HSI_VALUE;
    } break;
		case CLOCK_SOURCE_HSE:
    {
			result = HSE_VALUE;
    } break;
		case CLOCK_SOURCE_HCLK:
    {
			result = HAL_RCC_GetHCLKFreq();
    } break;
		case CLOCK_SOURCE_PCLK1:
    {
			result = HAL_RCC_GetPCLK1Freq();
    } break;
		case CLOCK_SOURCE_PCLK2:
    {
			result = HAL_RCC_GetPCLK2Freq();
    } break;
		case CLOCK_SOURCE_SYSCLK:
    {
			result = HAL_RCC_GetSysClockFreq();
    } break;
	}
	
	return result;
}

typedef u32 RESET_SOURCE;
enum 
{
	RESET_SOURCE_NONE,
	RESET_SOURCE_LOW_POWER,
	RESET_SOURCE_WWDG,
	RESET_SOURCE_IWDG,
	RESET_SOURCE_SOFTWARE,
	RESET_SOURCE_POR,
	RESET_SOURCE_NRST,
	RESET_SOURCE_BOR
};

// BOR detects a drop in operating voltage and triggers a reset before power is completely lost.
// POR is a reset device experiences on power up? (so a startup voltage) POR < BOR
// It prevents the device from running any software until a minimum level Vdd voltage threshold is met and the oscillator is stable.
// The levels they operate at are set by hardware in datasheet?

// window watchdog (reset at specific time in interval)
// independent watchdog (reset at any time in interval)

INTERNAL RESET_SOURCE 
get_reset_source(uint8_t clear_flags) 
{
  RESET_SOURCE result = RESET_SOURCE_NONE;

	if (RCC->CSR & RCC_CSR_LPWRRSTF) 
  {
		result = RESET_SOURCE_LOW_POWER;
	} 
  else if (RCC->CSR & RCC_CSR_WWDGRSTF)
  {
		result = RESET_SOURCE_WWDG;
	} 
  else if (RCC->CSR & RCC_CSR_WDGRSTF)
  {
		result = RESET_SOURCE_IWDG;
	} 
  else if (RCC->CSR & RCC_CSR_SFTRSTF)
  {
		result = RESET_SOURCE_SOFTWARE;
	} 
  else if (RCC->CSR & RCC_CSR_PORRSTF)
  {
		result = RESET_SOURCE_POR;
	} 
  else if (RCC->CSR & RCC_CSR_BORRSTF)
  {
		result = RESET_SOURCE_BOR;
	} 
  else if (RCC->CSR & RCC_CSR_PADRSTF)
  {
		result = RESET_SOURCE_NRST;
	}
	
	if (clear_flags) 
  {
		RCC->CSR = RCC_CSR_RMVF;
	}
	
	return result;
}

INTERNAL void 
TM_GENERAL_ForceHardFaultError(void) {
	/* Create hard-fault-function typedef */
	typedef void (*hff)(void);
	hff hf_func = 0;
	
	/* Call function at zero location in memory = HARDFAULT */
	hf_func();
}
