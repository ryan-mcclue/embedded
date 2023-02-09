// SPDX-License-Identifier: zlib-acknowledgement


// IMPLEMENTATION
__attribute__((weak)) int
open_resource(char *name)
{
  int fd = open(name);
  if (fd < 0) return -1
  else return fd;
}

// as mocks will often be resued, best to put in own file

// TESTING
extern int __real_open(const char *path, int flags, int mode);

int 
__wrap_open(const char *path, int flags, int mode)
{
  check_expected(path);

  return mock_type(int);
}

void
test_open_resource(void **state)
{
  // values are passed to mock functions as a queue 

  // expect allows us to set values for mock function parameters
  expect_string(__wrap_open, path, "/dev/i2c");

  // all calls to open passed to __wrap_open.

  // this will provide a value to mock_type(type)
  will_return(__wrap_open, -1);

  // cmocka call ordering:
  // expect_function_calls(name, num_times);

  int fd = __wrap_open(232);
  assert_int_equal();
}


