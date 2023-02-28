// SPDX-License-Identifier: zlib-acknowledgement

#include "stm32f429zitx-main.c"

#include "test-inc.h"

#include <stdio.h>
// A 'spike' is explorative code. Best to put in a separate branch.
// Main loop in executor function that returns true (so we can sleep or reset) 

// in test set-up you configure mocks to return particular values
// we are concerned with function calls, e.g. when x() is called and returns y I expect z() to be called
// mocks are required to maintain isolation with integration tests?
//
// gcc --Wl,--wrap=func; now have __wrap_func() and __real_func()

// boundary testing is putting in small, large numbers etc.
// system tests (automated or manual) is used to catch bugs (other testing is to prevent them).
// in essence, verify requirements are met in reality

void __wrap_stm32f429zitx_initialise(void)
{
  function_called();
}

void __wrap_HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
  function_called();
}

HAL_StatusTypeDef __wrap_HAL_UART_Init(UART_HandleTypeDef *huart)
{
  HAL_StatusTypeDef result = HAL_OK;

  function_called();

  return result; 
}

u32 __wrap_HAL_GetTick(void)
{
  u32 ms = 0;

  function_called();

  return ms;
}

GPIO_PinState __wrap_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  GPIO_PinState result = GPIO_PIN_RESET;

  function_called();

  return result;
}

void __wrap_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{

  function_called();
}



INTERNAL void
test_main_expected_to_initialise_hal_and_system_clock(void **state)
{
  expect_function_call(__wrap_HAL_Init);
  expect_function_call(__wrap_SystemClock_Config);

  assert_int_equal(testable_main(), 0);
}

// expected nomenclature for integration
// should nomenclature for unit
//INTERNAL void
//test_main_expected_to_initialise_permanent_and_temp_memory_arenas(void **state)
//{
//  // don't care what arguments passed in
//
//  // how to pass in a pointer here?
//  // will_return(mem_arena_allocate, );
//
//  expect_function_call(mem_arena_allocate, 1);
//  expect_function_call(initialise_global_temp_mem_arenas, 1);
//
//  return;
//  // TODO(Ryan):  
//}


INTERNAL void
test_s8_u32(void **state)
{
  b32 err = 0;

  assert_int_equal(s8_u32(s8_lit("0x1234"), &err), 0x1234);
  assert_int_equal(s8_u32(s8_lit("1234"), &err), 1234);
  assert_int_equal(s8_u32(s8_lit("0"), &err), 0);

  String8 large = s8_lit("0x20000078");
  u32 result = s8_u32(large, &err);

  assert_int_equal(result, 0x20000078);

}


int 
main(void)
{
	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_s8_u32),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;
}

