// SPDX-License-Identifier: zlib-acknowledgement

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include <limits.h>
#include <cmocka.h>

// peripherals.h with #define USB_TX 12 (pin names?)

extern int testable_main(void);

// unit tests/state-based testing for data structures, math, state machines, i.e. edge of project
// boundary testing is putting in small, large numbers etc.
// unit tests for driver files only required if not using tested BSP (just set/read register structs)

// mocking allows for interaction-based testing

// system tests (automated or manual) is used to catch bugs (other testing is to prevent them).
// in essence, verify requirements are met in reality

void
test_tem(void **state)
{
  assert_int_equal(testable_main(), 0);
}

// Is there any real point of testing main() as it will only be a few iterations anyway and simply testing a return code?
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

// mocks will utilise expect_ functions
// 1. Identify object to be mocked
// 2. What arguments and return values do you want to see for the mock object

// IMPORTANT(Ryan): To allow symbol redefinition
#if defined(TESTING)
#define INTERNAL
#define INLINE
#else
#define INTERNAL static
#define INLINE inline
#endif
int
__wrap_mock(char *path)
{
  check_expected_ptr(path);
  
  // mock_ptr_type(char *)
  int value = mock_type(int);

  return value;
}

void
test_with_mock(void **state)
{
  // this is what it will take
  expect_string(__wrap_mock, path, "hi");

  // this is what values are obtained from stack with mock_type(type) 
  will_return(__wrap_mock, 0);

  // so, seems we have two options in passing values to mock objects?

  // now call with arguments as we stated were expected
  int ret = some_func();
  assert_int_equal();
}

// a mock object will be setup in a caller function


// a mock is used when wanting to check if a function was called, e.g. expect_

// --wrap=open will redirect to __wrap_open

// may require specifying the weak attribute on source functions to be mocked 
// this allows redefinition of the symbol inside same translation unit due to unity build (perhaps good to put this in a macro for testing builds only?)

void
__wrap_led_init(void)
{
  function_called();  // IMPORTANT(Ryan): Put in every mock 
}

// TODO(Ryan): So far, interaction based testing just checking if things were called under particular conditions, e.g. returns true?
// This is achieved by replacing functions with mocks?
// So like unit tests, only test one source function at a time. The various logic flows yield the number of interaction tests?
// We are testing the testable attributes of function interactions, e.g. call counts, parameters, return types, ordering etc.

// Grouping tests into files will make separation easier with mocks.
// Put mocks into own file for reuse. led.c <-> mock_led.c etc.
// Will generate many test executables


// with mocking we are stacking up expectations that the mock will then unwind, i.e. creating a ledger to later debit
/* 1. paramater checking: 
     expect_string() <-> check_expected_ptr()
     expect_value() <-> check_expected()
   2. variables:
     will_return() <-> mock_type()
   3. call ordering:
     expect_function_call() <-> function_called() (will typically add a return value here too)
*/


void
init_peripherals(void)
{
  // give clocks, set resistors, turn startup leds on, init timer resolution 1/f and handler, etc.
  // IMPORTANT(Ryan): We could set interrupt handler in C instead of startup.s: (long *)SCB->VTOR[15] = handler; (this is better design)
}

////////////////
// MAIN
////////////////

void
test_MAIN_should_INITIALISE_LED(void)
{
  expect_function_call(led_init); // expect_function_calls()
  // will_return()

  expect_function_call(loop_exec);
  will_return(loop_exec, true);
  expect_function_call(loop_exec);
  will_return(loop_exec, true);
  expect_function_call(loop_exec);
  will_return(loop_exec, false);

  assert_int_equal(0, testable_main());
}

#if defined(TESTING)
void testable_main(void)
#else
void main(void)
#endif
{
  u32 last_tick = 0;

  init_peripherals(); // perhaps exec_init() and exec() to share common state?

  last_tick = get_timestamp();

  // we can now exit main loop say for sleep or reset
  while (loop_exec())
  {
    last_tick = timer_wait_remainder(last_tick, MAIN_TICK_MSEC);
  }
}

////////////////
// TIMER
////////////////

#if defined(TESTING)
u32 test_counter = 0;
#define WAIT_UNTIL_EQUAL(a, b) while (a++ != b) { ++test_counter; }
#else
#define WAIT_UNTIL_EQUAL(a, b) while (a != b) {}
#endif

u32
wait_remainder(u32 start, u32 ms)
{

}

void
wait_ms(u32 ms)
{
  u32 end_time = timer_counter + ms;
  WAIT_UNTIL_EQUAL(timer_counter, end_time);
}

////////////////
// DIGITAL
////////////////

void
test_LoopExec_Should_ToggleLED0WhenDigital0IsHigh(void)
{
  gpio_get_bits_expect_and_return(0x01);
  led_toggle_expect(LED0);

  loop_exec();
}
void
test_LoopExec_ShouldNot_ToggleLED0WhenDigital0IsLow(void)
{
  gpio_get_bits_expect_and_return(0x00);
  led_off_expect(LED0);

  loop_exec();
}

////////////////
// ANALOG
////////////////

void
analog_exec(void)
{
  for (u32 channel_i = 0; channel_i < 0; ++channel_i)
  {
    if (analog_channel_is_ready(channel_i))
    {
      value = analog_get_value(channel_i);
      // capture_buffer_put(value); first bits of buffer digital. next analog values 
      // corresponding, capture_buffer_get
      analog_filter_and_store(value, channel_i);
    }
  }
}

// probably better to use u32
u16 analog_values[ANALOG_NUM_CHANNELS];

// channels over digital pins
// filters employed with analog
void
test_loop_exec_should_turnled0on_whenanalog0greaterthananalog1(void)
{
  analog_get_channel_expect_and_return();
  led_on_expect();

  loop_exec();
}

void
test_loop_exec_shouldnot_updateanalog_unlessready(void)
{
  // implicit failing if other functions called outside of what we say
  // so, we are saying don't expect other functions to be called (so put function_called() in every mock?)
  analog_isready_expect_and_return(0, OK);
  analog_getreading_expect_and_return(0, 0x1234);
  analog_addfilterreading_expect_and_return(0, 0x1234);
  analog_isready_expect_and_return(1, NO);

  loop_exec();
}

////////////////
// SERIAL
////////////////

// A serial driver will typically just populate bytes in fifo queues?
void
serial_exec(void)
{
  process_raw_serial(); 

  Message msg;
  if (check_for_msg_from_serial(&msg))
  {

  }
}

void
test_should_ignore_unknown_messages(void)
{
  msg.cmd = 'u';
  expect_any(check_for_msg, msg);
}

// TODO(Ryan): Confused with USB-3 and UART. If can only send one byte at a time, how faster?
// All embedded serial ports are UART, so slower than USB-3 protocol on desktops?
void
test_should_read(void)
{
  usb_driver_okaytoread_expect_and_return(true);
  usb_driver_getbyte_expect_and_return('V');
  parser_add_char_expect_and_return('V')

  check_for_msg();
}

void
test_should_write(void)
{
  parser_add_msg_expect();
  usb_driver_add_char_expect();
  usb_driver_add_char_expect();
  usb_driver_add_char_expect();
  usb_driver_send_msg_expect();

  // if (serial_readable() && !buffer_isempty())
  // i.e. how to handle short circuit evaluation
  ignore_function_calls(buffer_isempty)

  test_assert_equal(send_response(&msg), STATUS_OK);
}

// INTERACTION TEST CASES (makes program more functional and enums):
// 1. Some input triggers an output, e.g. button sets LED on

// test a loop that theoretically runs forever:
// 1. wrap loop into a function that returns boolean used in condition, e.g. while (loop_iteration()) {} 

// getters/setters/is_ready/is_empty/is_full useful for interaction based testing
// put function_called() in every mock to allow for implicit failing?
// like unit tests, just check control flow
// better to do assert_hex_equal() as more informative?

// TODO(Ryan): perhaps in Q&A, how to write system/integration tests?
