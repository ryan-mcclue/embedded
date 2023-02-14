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

void __wrap_HAL_Init(void)
{
  function_called();
}

void __wrap_SystemClock_Config(void)
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


int 
main(void)
{
	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_main_expected_to_initialise_hal_and_system_clock),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;
}

