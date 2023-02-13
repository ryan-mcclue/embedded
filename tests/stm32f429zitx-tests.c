// SPDX-License-Identifier: zlib-acknowledgement

#include "test-inc.h"

int zero_func(void)
{
  return 0;
}

void
test_func(void **state)
{
  assert_int_equal(zero_func(), 0);
}

#include <stdio.h>

int 
main(void)
{
  printf("%s, %s, %d\n", BUILD_VERSION_STR, BUILD_DATE_STR, RAM_SIZE_INT);

	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_func),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;
}

