// SPDX-License-Identifier: zlib-acknowledgement

// more useful for dual core
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
force_hard_fault(void)
{
	typedef void (*hff)(void);
	hff hf_func = 0;
	hf_func();
}

typedef u32 BOR_LEVEL;
enum 
{
  BOR_LEVEL_OFF = OB_BOR_OFF, 
	BOR_LEVEL_1 = OB_BOR_LEVEL1,
	BOR_LEVEL_2 = OB_BOR_LEVEL2,
	BOR_LEVEL_3 = OB_BOR_LEVEL3
};

typedef enum {
	TM_BOR_Result_Ok = 0x00, /*!< Everything OK */
	TM_BOR_Result_Error      /*!< An error has occurred */
} TM_BOR_Result_t;

// flash option bytes area of flash used to control certain peripherals
// TODO: command line programming of just flash option bytes 

// fast SPI tft: https://www.youtube.com/watch?v=oWx1-WmTwag 
// https://github.com/maudeve-it/ST7735S-STM32

// fast I2C eeprom: https://www.youtube.com/watch?v=Rd5CeMbla5g  

// PVD for detecting higher voltages than BOR (so perhaps write to flash in PVD?)
// need a certain capacitance and inductance of power supply to allow for EEPROM writing?
// https://www.youtube.com/watch?v=AHBGlCDGqhE

INTERNAL BOR_LEVEL 
get_brownout_level(void)
{
	FLASH_OBProgramInitTypeDef flash_ob_handle = ZERO_STRUCT;
	
	HAL_FLASHEx_OBGetConfig(&flash_ob_handle);

	return (BOR_LEVEL)flash_ob_handle.BORLevel;
}

INTERNAL TM_BOR_Result_t 
TM_BOR_Set(BOR_LEVEL bor_level) {
	HAL_StatusTypeDef status;

  // flash option bytes
	FLASH_OBProgramInitTypeDef FLASH_Handle;
	
	/* Check current BOR value */
	if (TM_BOR_Get() != bor_level) {
		/* Set new value */

		/* Select the desired V(BOR) Level */
		FLASH_Handle.BORLevel = (uint32_t)BORValue;
		FLASH_Handle.OptionType = OPTIONBYTE_BOR;

		/* Unlocks the option bytes block access */
		HAL_FLASH_OB_Unlock();
		
		/* Set value */
		HAL_FLASHEx_OBProgram(&FLASH_Handle); 

		/* Launch the option byte loading */
		status = HAL_FLASH_OB_Launch();
		
		/* Lock access to registers */
		HAL_FLASH_OB_Lock();
		
		/* Check success */
		if (status != HAL_OK) {
			/* Return error */
			return TM_BOR_Result_Error;
		}
	}
	
	/* Return OK */
	return TM_BOR_Result_Ok;
}

/* NVIC preemption priority */
#ifndef PVD_NVIC_PRIORITY
#define PVD_NVIC_PRIORITY      0x04
#endif

/* NVIC subpriority */
#ifndef PVD_NVIC_SUBPRIORITY
#define PVD_NVIC_SUBPRIORITY   0x00
#endif

typedef enum {
	TM_PVD_Trigger_Rising = PWR_PVD_MODE_IT_RISING,                /*!< PVD will trigger interrupt when voltage rises above treshold */
	TM_PVD_Trigger_Falling = PWR_PVD_MODE_IT_FALLING,              /*!< PVD will trigger interrupt when voltage falls below treshold */
	TM_PVD_Trigger_Rising_Falling = PWR_PVD_MODE_IT_RISING_FALLING /*!< PVD will trigger interrupt when voltage rises or falls above/below treshold */
} TM_PVD_Trigger_t;

/**
 * @brief  PVD levels for interrupt triggering
 * @note   Check datasheets for proper values for voltages
 */
typedef enum {
	TM_PVD_Level_0 = PWR_PVDLEVEL_0, /*!< PVD level 0 is used as treshold value */
	TM_PVD_Level_1 = PWR_PVDLEVEL_1, /*!< PVD level 1 is used as treshold value */
	TM_PVD_Level_2 = PWR_PVDLEVEL_2, /*!< PVD level 2 is used as treshold value */
	TM_PVD_Level_3 = PWR_PVDLEVEL_3, /*!< PVD level 3 is used as treshold value */
	TM_PVD_Level_4 = PWR_PVDLEVEL_4, /*!< PVD level 4 is used as treshold value */
	TM_PVD_Level_5 = PWR_PVDLEVEL_5, /*!< PVD level 5 is used as treshold value */
	TM_PVD_Level_6 = PWR_PVDLEVEL_6, /*!< PVD level 6 is used as treshold value */
	TM_PVD_Level_7 = PWR_PVDLEVEL_7  /*!< PVD level 7 is used as treshold value */
} TM_PVD_Level_t;


void TM_PVD_Enable(TM_PVD_Level_t Level, TM_PVD_Trigger_t Trigger) {
	PWR_PVDTypeDef ConfigPVD;
	
	/* Enable PWR clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	
	/* Set interrupt to NVIC */
	HAL_NVIC_SetPriority(PVD_IRQn, PVD_NVIC_PRIORITY, PVD_NVIC_SUBPRIORITY);
	
	/* Enable interrupt */
	HAL_NVIC_EnableIRQ(PVD_IRQn);
	
	/* Set level and mode */
	ConfigPVD.PVDLevel = (uint32_t)Level;
	ConfigPVD.Mode = Trigger;
	
	/* Config and enable PVD */
	HAL_PWR_ConfigPVD(&ConfigPVD);
	
	/* Enable PVD */
	HAL_PWR_EnablePVD();
}

void TM_PVD_Disable(void) {
	/* Disable PVD */
	HAL_PWR_DisablePVD();
	
	/* Disable EXTI interrupt for PVD */
	__HAL_PWR_PVD_EXTI_DISABLE_IT();

	/* Disable NVIC */
	NVIC_DisableIRQ(PVD_IRQn);
}


void PVD_IRQHandler(void) {
	/* Call user function if needed */
	if (__HAL_PWR_PVD_EXTI_GET_FLAG() != RESET) {
#if defined(PWR_CSR_PVDO)	
		/* Call user function with status */
		TM_PVD_Handler((PWR->CSR & PWR_CSR_PVDO) ? 1 : 0);
#endif
#if defined(PWR_CSR1_PVDO)
		/* Call user function with status */
		TM_PVD_Handler((PWR->CSR1 & PWR_CSR1_PVDO) ? 1 : 0);
#endif
		/* Clear PWR EXTI pending bit */
		__HAL_PWR_PVD_EXTI_CLEAR_FLAG();
	}
}

// IMPORTANT(Ryan): STM32 also has HAL and LL 'driver description' pdfs

void 
TM_PVD_Handler(uint8_t status) {
	/* Check status */
	if (status) {
		/* Power is below trigger voltage */
		TM_DISCO_LedOn(LED_ALL);
	} else {
		/* Power is above trigger voltage */
		TM_DISCO_LedOff(LED_ALL);
	}

  eeprom_write(addr, data, sizeof(data));
}

u8 counter;

void 
main(void)
{
  // IMPORTANT(Ryan): When looking at EEPROM characteristics: 
  //   * page write time size
  //   * lowest operating voltage
  char text[EEPROM_PAGE_LEN];
  eeprom_read(addr, text);

  // set at highest level to detect power failer earliest
	/* Init Power Voltage Detector with rising and falling interrupt */
	TM_PVD_Enable(TM_PVD_Level_3, TM_PVD_Trigger_Rising_Falling);
}

#define MCU_GET_SIGNATURE()     (DBGMCU->IDCODE & 0x00000FFF)
#define MCU_GET_REVISION()      ((DBGMCU->IDCODE >> 16) & 0x0000FFFF)

#define MCU_FLASH_SIZE_ADDRESS (0x1FFF7A22)
#define FLASH_SIZE_KB     (*(volatile u16 *)(MCU_FLASH_SIZE_ADDRESS))

#define MCU_ID_ADDRESS (0x1FFF7A10)
#define MCU_ID_U8      ((*(volatile u8 *)(MCU_ID_ADDRESS)
#define MCU_ID_U16     ((*(volatile u16 *)(MCU_ID_ADDRESS)
#define MCU_ID_U32     ((*(volatile u32 *)(MCU_ID_ADDRESS)

typedef struct {
	float Load;      /*!< CPU load percentage */
	uint8_t Updated; /*!< Set to 1 when new CPU load is calculated */
	uint32_t WCNT;   /*!< Number of working cycles in one period. Meant for private use */
	uint32_t SCNT;   /*!< Number of sleeping cycles in one period. Meant for private use */
} TM_CPULOAD_t;


uint8_t TM_CPULOAD_GoToSleepMode(TM_CPULOAD_t* CPU_Load) {
	uint32_t t;
	static uint32_t l = 0;
	static uint32_t WorkingTime = 0;
	static uint32_t SleepingTime = 0;
	uint8_t irq_status;
	
	/* Add to working time */
	WorkingTime += DWT->CYCCNT - l;
	
	/* Save count cycle time */
	t = DWT->CYCCNT;
	
	/* Get interrupt status */
	irq_status = __get_PRIMASK();
	
	/* Disable interrupts */
	__disable_irq();
	
	/* Go to sleep mode */
	/* Wait for wake up interrupt, systick can do it too */
	__WFI();
	
	/* Increase number of sleeping time in CPU cycles */
	SleepingTime += DWT->CYCCNT - t;
	
	/* Save current time to get number of working CPU cycles */
	l = DWT->CYCCNT;
	
	/* Enable interrupts, process/execute an interrupt routine which wake up CPU */
	if (!irq_status) {
		__enable_irq();
	}
	
	/* Reset flag */
	CPU_Load->Updated = 0;
	
	/* Every 1000ms print CPU load via USART */
	if ((SleepingTime + WorkingTime) >= HAL_RCC_GetHCLKFreq()) {
		/* Save values */
		CPU_Load->SCNT = SleepingTime;
		CPU_Load->WCNT = WorkingTime;
		CPU_Load->Load = ((float)WorkingTime / (float)(SleepingTime + WorkingTime) * 100);
		CPU_Load->Updated = 1;
		
		/* Reset time */
		SleepingTime = 0;
		WorkingTime = 0;
	}
	
	/* Return updated status */
	return CPU_Load->Updated;
}

void main(void)
{
/*
 *What do you want to do with "other interrupts"? If you are planning to sleep until one specific interrupt occurs, then you should simply disable all other interrupts, execute WFI and then re-enable everything when you wake up.
 */
  // __ISB
  // https://chiselapp.com/user/mangoa01/repository/bottom-up/uv/docs/book/micro_bottom_up.pdf

	while (1) {
		/* Check if CPU LOAD variable is updated */
		if (CPU_LOAD.Updated) {
			/* Print to user */
			printf("W: %u; S: %u; Load: %5.2f\n", CPU_LOAD.WCNT, CPU_LOAD.SCNT, CPU_LOAD.Load);
		}

		/* Go low power mode, sleep mode until next interrupt (Systick or anything else), measure CPU load */
		TM_CPULOAD_GoToSleepMode(&CPU_LOAD);
  }
}
