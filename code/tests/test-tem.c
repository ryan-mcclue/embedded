// SPDX-License-Identifier: zlib-acknowledgement

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>
#include <cmocka.h>

#include <stdio.h>

extern void qemu_arm_exit(void);

extern int testable_main(void);

// A 'spike' is explorative code. Best to put in a separate branch.

/*
 * REFACTORING FOR INTERACTION TESTABILITY:
 * · Driver initialisation (init_drivers()) --> testing init_drivers() expect_init_gpio() etc.
 * · Main loop in executor function that returns true (so we can sleep or reset) 
 *
 * · For a collection of related ports (e.g. gpio leds), 
 *   have a function to return a bitmask of whether they are on (so create a file for them).
 *   So, a bitmask setter and getter
 * · Use enums to map to related port numbers, e.g. LED0, LED1
 */

// #include "mock-led.c"

// For testing main function, just lay out what functions you expect to be called? 
void
test_tem(void **state)
{
  // init_drivers_Expect()
  //
  // tem_exec_ExpectAndReturn(TRUE);
  // tem_exec_ExpectAndReturn(TRUE);
  // tem_exec_ExpectAndReturn(FALSE);
  assert_int_equal(testable_main(), 0);

  // this won't be flexible to function name changes......
  //
  // checking if led is on is really just checking register states
  // this is not useful if using a hal, as we are just testing their framework

  // with testing, unit tests for 'edge modules' and test their state (a whole lot of jargon for the same thing!)
  // naming with 'should' terminology?
  //
  // integration tests for interworking parts and test their interaction 
  // naming with 'expect' terminology? (these expectations are collection of asserts) 
  // in test set-up you configure mocks to return particular values
  // we are concerned with function calls, e.g. when x() is called and returns y I expect z() to be called
  // mocks are required to maintain isolation with integration tests?
  //
  // gcc --Wl,--wrap=func; now have __wrap_func() and __real_func()

}

int 
main(void)
{
	const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_tem),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  qemu_arm_exit();

  return cmocka_res;
}
