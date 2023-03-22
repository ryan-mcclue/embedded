// SPDX-License-Identifier: zlib-acknowledgement

typedef enum {
	TM_GENERAL_Clock_HSI,    /*!< High speed internal clock */
	TM_GENERAL_Clock_HSE,    /*!< High speed external clock */
	TM_GENERAL_Clock_SYSCLK, /*!< System core clock */
	TM_GENERAL_Clock_PCLK1,  /*!< PCLK1 (APB1) peripheral clock */
	TM_GENERAL_Clock_PCLK2,  /*!< PCLK2 (APB2) peripheral clock */
	TM_GENERAL_Clock_HCLK    /*!< HCLK (AHB1) high speed clock */
} TM_GENERAL_Clock_t;

/**
 * @brief  All possible reset sources
 */
typedef enum {
	TM_GENERAL_ResetSource_None = 0x00,     /*!< No reset source detected. Flags are already cleared */
	TM_GENERAL_ResetSource_LowPower = 0x01, /*!< Low-power management reset occurs */
	TM_GENERAL_ResetSource_WWDG = 0x02,     /*!< Window watchdog reset occurs */
	TM_GENERAL_ResetSource_IWDG = 0x03,     /*!< Independent watchdog reset occurs */
	TM_GENERAL_ResetSource_Software = 0x04, /*!< Software reset occurs */
	TM_GENERAL_ResetSource_POR = 0x05,      /*!< POR/PDR reset occurs */
	TM_GENERAL_ResetSource_PIN = 0x06,      /*!< NRST pin is set to low by hardware reset, hardware reset */
	TM_GENERAL_ResetSource_BOR = 0x07,      /*!< BOR reset occurs */
} TM_GENERAL_ResetSource_t;

/**
 * @brief  Float number operation structure
 */
typedef struct {
	int32_t Integer;  /*!< Integer part of float number */
	uint32_t Decimal; /*!< Decimal part of float number */
} TM_GENERAL_Float_t;

/**
 * @}
 */

/**
 * @defgroup TM_GENERAL_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Performs a system reset
 * @note   Before system will be reset, @ref TM_GENERAL_SoftwareResetCallback() will be called,
 *         where you can do important stuff if necessary
 * @param  None
 * @retval None
 */
void TM_GENERAL_SystemReset(void);

/**
 * @brief  Gets reset source why system was reset
 * @param  reset_flags: After read, clear reset flags
 *            - 0: Flags will stay untouched
 *            - > 0: All reset flags will reset
 * @retval Member of @ref TM_GENERAL_ResetSource_t containing reset source
 */
TM_GENERAL_ResetSource_t TM_GENERAL_GetResetSource(uint8_t reset_flags);

/**
 * @brief  Disables all interrupts in system
 * @param  None
 * @retval None
 */
void TM_GENERAL_DisableInterrupts(void);

/**
 * @brief  Enables interrupts in system.
 * @note   This function has nesting support. This means that if you call @ref TM_GENERAL_DisableInterrupts() 4 times,
 *         then you have to call this function also 4 times to enable interrupts.
 * @param  None
 * @retval Interrupt enabled status:
 *            - 0: Interrupts were not enabled
 *            - > 0: Interrupts were enabled
 */
uint8_t TM_GENERAL_EnableInterrupts(void);

/**
 * @brief  Checks if code execution is inside active IRQ
 * @param  None
 * @retval IRQ Execution status:
 *            - 0: Code execution is not inside IRQ, thread mode
 *            - > 0: Code execution is inside IRQ, IRQ mode
 * @note   Defines as macro for faster execution
 */
#define TM_GENERAL_IsIRQMode()               (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)

/**
 * @brief  Gets specific clock speed value from STM32F4xx device
 * @param  clock: Clock type you want to know speed for. This parameter can be a value of @ref TM_GENERAL_Clock_t enumeration
 * @retval Clock speed in units of hertz
 */
uint32_t TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_t clock);

/**
 * @brief  Gets system clock speed in units of MHz
 * @param  None
 * @retval None
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_GetSystemClockMHz()       ((uint16_t)(SystemCoreClock * (float)0.000001))

/**
 * @brief  Enables DWT counter in Cortex-M4 core
 * @param  None
 * @retval DWT Status:
 *            - 0: DWT has not started, hardware/software reset is required
 *            - > 0: DWT has started and is ready to use
 * @note   It may happen, that DWT counter won't start after reprogramming device.
 *         This happened to me when I use onboard ST-Link on Discovery or Nucleo boards.
 *         When I used external debugger (J-Link or ULINK2) it worked always without problems.
 *         If your DWT doesn't start, you should perform software/hardware reset by yourself.
 */
uint8_t TM_GENERAL_DWTCounterEnable(void);

/**
 * @brief  Disables DWT counter in Cortex-M4 core
 * @param  None
 * @retval None
 * @note   Defined as macro for faster execution
 */
#if !defined(STM32F0xx)
#define TM_GENERAL_DWTCounterDisable()       (DWT->CTRL &= ~0x00000001)
#endif


/**
 * @brief  Gets current DWT counter value
 * @param  None
 * @retval DWT counter value
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_DWTCounterGetValue()      (DWT->CYCCNT)

/**
 * @brief  Sets DWT counter value
 * @param  x: Value to be set to DWT counter
 * @retval None
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_DWTCounterSetValue(x)     (DWT->CYCCNT = (x))

/**
 * @brief  Delays for amount of microseconds using DWT counter
 * @param  micros: Number of micro seconds for delay 
 * @retval None
 * @note   DWT Counter HAVE to be initialized first using @ref TM_GENERAL_EnableDWTCounter()
 */
static __INLINE void TM_GENERAL_DWTCounterDelayus(uint32_t micros) {
	uint32_t c = TM_GENERAL_DWTCounterGetValue();
	
	/* Calculate clock cycles */
	micros *= (SystemCoreClock / 1000000);
	micros -= 12;
	
	/* Wait till done */
	while ((TM_GENERAL_DWTCounterGetValue() - c) < micros);
}

/**
 * @brief  Delays for amount of milliseconds using DWT counter
 * @param  millis: Number of micro seconds for delay 
 * @retval None
 * @note   DWT Counter HAVE to be initialized first using @ref TM_GENERAL_EnableDWTCounter()
 */
static __INLINE void TM_GENERAL_DWTCounterDelayms(uint32_t millis) {
	uint32_t c = TM_GENERAL_DWTCounterGetValue();
	
	/* Calculate clock cycles */
	millis *= (SystemCoreClock / 1000);
	millis -= 12;
	
	/* Wait till done */
	while ((TM_GENERAL_DWTCounterGetValue() - c) < millis);
}

/**
 * @brief  Checks if number is odd or even
 * @param  number: Number to check if it is odd or even
 * @retval Is number even status:
 *            - 0: Number is odd
 *            - > 0: Number is even
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_IsNumberEven(number)          ((number & 1) == 0)

/**
 * @brief  Converts float coded number into integer and decimal part
 * @param  *Float_Struct: Pointer to empty @ref TM_GENERAL_Float_t to store result into
 * @param  Number: Float number to convert
 * @param  decimals: Number of decimal places for conversion, maximum 9 decimal places
 * @retval None
 * @note   Example: You have number 15.002 in float format.
 *            - You want to split this to integer and decimal part with 6 decimal places.
 *            - Call @ref TM_GENERAL_ConvertFloat(&Float_Struct, 15.002, 6);
 *            - Result will be: Integer: 15; Decimal: 2000 (0.002 * 10^6)
 */
void TM_GENERAL_ConvertFloat(TM_GENERAL_Float_t* Float_Struct, float Number, uint8_t decimals);

/**
 * @brief  Round float number to nearest number with custom number of decimal places
 * @param  Number: Float number to round
 * @param  decimals: Number of decimal places to round, maximum 9 decimal places
 * @retval Rounded float number
 */
float TM_GENERAL_RoundFloat(float Number, uint8_t decimals);

/**
 * @brief  Checks if number is power of 2
 * @note   It can be used to determine if number has more than just one bit set
 *         If only one bit is set, function will return > 0 because this is power of 2.
 * @param  number: Number to check if it is power of 2
 * @retval Is power of 2 status:
 *            - 0: Number if not power of 2
 *            - > 0: Number is power of 2
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_IsNumberPowerOfTwo(number)    (number && !(number & (number - 1)))

/**
 * @brief  Calculates next power of 2 from given number
 * @param  number: Input number to be calculated
 * @retval Number with next power of 2
 *         Example:
 *            - Input number: 450
 *            - Next power of 2 is: 512 = 2^9
 */
uint32_t TM_GENERAL_NextPowerOf2(uint32_t number);

/**
 * @brief  Forces processor to jump to Hard-fault handler
 * @note   Function tries to call function at zero location in memory which causes hard-fault
 * @param  None
 * @retval None
 */
void TM_GENERAL_ForceHardFaultError(void);

/**
 * @brief  System reset callback.
 * @note   Function is called before software reset occurs.
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void TM_GENERAL_SystemResetCallback(void);

/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif


/**	
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen MAJERLE
 * |  
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software, 
 * | and to permit persons to whom the Software is furnished to do so, 
 * | subject to the following conditions:
 * | 
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * | 
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32_general.h"

/* System speed in MHz */
uint16_t GENERAL_SystemSpeedInMHz = 0;
static uint16_t InterruptDisabledCount = 0;

/* Private functions */
static uint32_t x_na_y(uint32_t x, uint8_t y) {
	uint32_t output = 1;
	
	/* Make a "power" multiply */
	while (y--) {
		output *= x;
	}
	
	/* Return output value */
	return output;
}

void TM_GENERAL_DisableInterrupts(void) {
	/* Disable interrupts */
	__disable_irq();
	
	/* Increase number of disable interrupt function calls */
	InterruptDisabledCount++;
}

uint8_t TM_GENERAL_EnableInterrupts(void) {
	/* Decrease number of disable interrupt function calls */
	if (InterruptDisabledCount) {
		InterruptDisabledCount--;
	}
	
	/* Check if we are ready to enable interrupts */
	if (!InterruptDisabledCount) {
		/* Enable interrupts */
		__enable_irq();
	}
	
	/* Return interrupt enabled status */
	return !InterruptDisabledCount;
}

void TM_GENERAL_SystemReset(void) {
	/* Call user callback function */
	TM_GENERAL_SystemResetCallback();
	
	/* Perform a system software reset */
	NVIC_SystemReset();
}

uint32_t TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_t clock) {
	uint32_t c = 0;

	/* Return clock speed */
	switch (clock) {
		case TM_GENERAL_Clock_HSI:
			c = HSI_VALUE;
			break;
		case TM_GENERAL_Clock_HSE:
			c = HSE_VALUE;
			break;
		case TM_GENERAL_Clock_HCLK:
			c = HAL_RCC_GetHCLKFreq();
			break;
		case TM_GENERAL_Clock_PCLK1:
			c = HAL_RCC_GetPCLK1Freq();
			break;
		case TM_GENERAL_Clock_PCLK2:
			c = HAL_RCC_GetPCLK2Freq();
			break;
		case TM_GENERAL_Clock_SYSCLK:
			c = HAL_RCC_GetSysClockFreq();
			break;
		default:
			break;
	}
	
	/* Return clock */
	return c;
}

TM_GENERAL_ResetSource_t TM_GENERAL_GetResetSource(uint8_t reset_flags) {
	TM_GENERAL_ResetSource_t source = TM_GENERAL_ResetSource_None;

	/* Check bits */
	if (RCC->CSR & RCC_CSR_LPWRRSTF) {
		source = TM_GENERAL_ResetSource_LowPower;
	} else if (RCC->CSR & RCC_CSR_WWDGRSTF) {
		source = TM_GENERAL_ResetSource_WWDG;
#if defined(STM32F4xx)
	} else if (RCC->CSR & RCC_CSR_WDGRSTF) {
#else
	} else if (RCC->CSR & RCC_CSR_IWDGRSTF) {
#endif
		source = TM_GENERAL_ResetSource_IWDG;
	} else if (RCC->CSR & RCC_CSR_SFTRSTF) {
		source = TM_GENERAL_ResetSource_Software;
	} else if (RCC->CSR & RCC_CSR_PORRSTF) {
		source = TM_GENERAL_ResetSource_POR;
	} else if (RCC->CSR & RCC_CSR_BORRSTF) {
		source = TM_GENERAL_ResetSource_BOR;
#if defined(STM32F4xx)
	} else if (RCC->CSR & RCC_CSR_PADRSTF) {
#else
	} else if (RCC->CSR & RCC_CSR_PINRSTF) {
#endif		
		source = TM_GENERAL_ResetSource_PIN;
	}
	
	/* Check for clearing flags */
	if (reset_flags) {
		RCC->CSR = RCC_CSR_RMVF;
	}
	
	/* Return source */
	return source;
}

#if !defined(STM32F0xx)
uint8_t TM_GENERAL_DWTCounterEnable(void) {
	uint32_t c;
	
	/* Set clock speed if not already */
	if (GENERAL_SystemSpeedInMHz == 0) {
		/* Get clock speed in MHz */
		GENERAL_SystemSpeedInMHz = TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_SYSCLK) / 1000000;
	}
	
    /* Enable TRC */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;

#if defined(STM32F7xx)
    /* Unclock DWT timer */
    DWT->LAR = 0xC5ACCE55;
#endif

    /* Enable counter */
    DWT->CTRL &= ~0x00000001;
    DWT->CTRL |=  0x00000001;
	
    /* Reset counter */
    DWT->CYCCNT = 0;
	
	/* Check if DWT has started */
	c = DWT->CYCCNT;
	
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
	
	/* Return difference, if result is zero, DWT has not started */
	return (DWT->CYCCNT - c);
}
#endif

void TM_GENERAL_ConvertFloat(TM_GENERAL_Float_t* Float_Struct, float Number, uint8_t decimals) {
	/* Check decimals */
	if (decimals > 9) {
		decimals = 9;
	}
	
	/* Get integer part */
	Float_Struct->Integer = (int32_t)Number;
	
	/* Get decimal part */
	if (Number < 0) {
		Float_Struct->Decimal = (int32_t)((float)(Float_Struct->Integer - Number) * x_na_y(10, decimals));
	} else {
		Float_Struct->Decimal = (int32_t)((float)(Number - Float_Struct->Integer) * x_na_y(10, decimals));
	}
}

float TM_GENERAL_RoundFloat(float Number, uint8_t decimals) {
	float x;
		
	/* Check decimals */
	if (decimals > 9) {
		decimals = 9;
	}
	
	x = x_na_y(10, decimals);
	
	/* Make truncating */
	if (Number > 0) {
		return (float)(Number * x + (float)0.5) / x;
	} 
	if (Number < 0) {
		return (float)(Number * x - (float)0.5) / x;
	}
	
	/* Return number */
	return 0;
}

uint32_t TM_GENERAL_NextPowerOf2(uint32_t number) {
	/* Check number */
	if (number <= 1) {
		return 1;
	}
	
	/* Do some bit operations */
	number--;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number++;
	
	/* Return calculated number */
	return number;
}

void TM_GENERAL_ForceHardFaultError(void) {
	/* Create hard-fault-function typedef */
	typedef void (*hff)(void);
	hff hf_func = 0;
	
	/* Call function at zero location in memory = HARDFAULT */
	hf_func();
}

__weak void TM_GENERAL_SystemResetCallback(void) {
	/* NOTE: This function should not be modified, when the callback is needed,
            the TM_GENERAL_SystemResetCallback could be implemented in the user file
	*/
}


void main(void)
{
	printf("System reset source: %d\n", (uint8_t)TM_GENERAL_GetResetSource(1));
		TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_SYSCLK),
		TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_PCLK1)

			TM_GENERAL_SystemReset();
}

/* Called before system reset */
void TM_GENERAL_SystemResetCallback(void) {
	/* Print to user */
	printf("System reset callback\n");
	
	/* A little delay */
	Delayms(10);
}
