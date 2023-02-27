// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "test-inc.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <time.h>


#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B57600

GLOBAL int global_serial_fd;
GLOBAL String8 global_serial_output;
GLOBAL size_t global_serial_output_len;

INTERNAL u64
get_ms(void)
{
  u64 result = 0;

  struct timespec time_spec = {0};
  clock_gettime(CLOCK_MONOTONIC_RAW, &time_spec);

  result = (time_spec.tv_sec * 1000LL) + (time_spec.tv_nsec / 1000000.0f);

  return result;
}

void
sleep_ms(int ms)
{
  struct timespec sleep_time = {0};
  sleep_time.tv_nsec = ms * 1000000;
  struct timespec leftover_sleep_time = {0};
  nanosleep(&sleep_time, &leftover_sleep_time);
}


INTERNAL void
writex(int fd, void *buf, size_t count)
{
  size_t bytes_written = write(fd, buf, count);
  if (bytes_written == -1)
  {
    fprintf(stderr, "Error: write failed (%s)\n", strerror(errno));
    exit(1);
  }
  if (bytes_written != count)
  {
    fprintf(stderr, "Warning: write failed to write all bytes (%s)\n", strerror(errno));
  }
}

INTERNAL size_t
readx(int fd, void *buf, size_t count)
{
  size_t bytes_read = read(fd, buf, count);
  if (bytes_read == -1)
  {
    fprintf(stderr, "Error: read failed (%s)\n", strerror(errno));
    exit(1);
  }

  return bytes_read;
}

INTERNAL void 
execute_test(String8 input, String8 output, u32 max_wait_time_ms)
{
	writex(global_serial_fd, input.str, input.size);

  u32 bytes_expected = output.size;

  u64 start_time_ms = get_ms();
  u32 bytes_available = 0;
  while (bytes_available != bytes_expected)
  {
    ioctl(global_serial_fd, FIONREAD, &bytes_available);
   
    if (get_ms() - start_time_ms >= max_wait_time_ms)
    {
      break;
    }
  }

  assert_int_equal(bytes_available, bytes_expected);
  
	size_t bytes_read = readx(global_serial_fd, 
                            global_serial_output.str, 
                            bytes_expected);
  global_serial_output.str[bytes_read] = '\0';
  global_serial_output.size = bytes_read;

  // NOTE(Ryan): The output string must be null terminated
  assert_string_equal(output.str, global_serial_output.str);
}

INTERNAL void
test_console(void **state)
{
  execute_test(s8_lit("test\n"), s8_lit("Test\n"), 500);
}

INTERNAL void
disable_logging(void)
{
  String8 input = s8_lit("-\n");
	writex(global_serial_fd, input.str, input.size);

  sleep_ms(100);

  // NOTE(Ryan): Remove any unread 'logging' data
  tcflush(global_serial_fd, TCIFLUSH);
}


INTERNAL void
initialise_serial_port(char *serial_port, u32 baud_rate, u32 read_timeout)
{
  // IMPORTANT(Ryan): If specifying non-blocking arguments in here can affect VTIM
  // and result in 'resource temporarily unavaiable'
	global_serial_fd = open(serial_port, O_RDWR);
	if (global_serial_fd < 0) 
  {
    fprintf(stderr, "Error: opening serial port %s failed (%s)\n", serial_port, strerror(errno));
		exit(1);
	}

	struct termios serial_options = {0};
	if (tcgetattr(global_serial_fd, &serial_options) == -1)
  {
    fprintf(stderr, "Error: obtaining original serial port settings failed (%s)\n", strerror(errno));
		exit(1);
  }

  // NOTE(Ryan): Important to alter specific bits to avoid undefined behaviour 

  // CONTROL MODES
  // disable parity
  serial_options.c_cflag &= ~PARENB;

  // one stop bit
  serial_options.c_cflag &= ~CSTOPB;

  serial_options.c_cflag &= ~CSIZE;
  serial_options.c_cflag |= CS8;

  // disable hardware flow control
  serial_options.c_cflag &= ~CRTSCTS;

  // allow reads
  // no carrier detect if removed
  serial_options.c_cflag |= (CREAD | CLOCAL);

  // LOCAL MODES
  // disable canonical mode so as to recieve all raw bytes and say not interpret backspace specially
  serial_options.c_lflag &= ~ICANON;

  serial_options.c_lflag &= ~ECHO; // Disable echo
  serial_options.c_lflag &= ~ECHOE; // Disable erasure
  serial_options.c_lflag &= ~ECHONL; // Disable new-line echo

  // disable interpretation of certain signals
  serial_options.c_lflag &= ~ISIG;

  // INPUT MODES
  // disable software flow control
  serial_options.c_iflag &= ~(IXON | IXOFF | IXANY);
  // disble special handling of bytes
  serial_options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

  // OUTPUT MODES
  // disable special handling of bytes
  serial_options.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  serial_options.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

  // whther blocking or not determined with VMIN and VTIME
  serial_options.c_cc[VTIME] = (read_timeout * 10);    // Wait for up to deciseconds, returning as soon as any data is received.
  serial_options.c_cc[VMIN] = 0;

  cfsetspeed(&serial_options, BAUD_RATE);

	if (tcsetattr(global_serial_fd, TCSANOW, &serial_options) == -1)
  {
    fprintf(stderr, "Error: setting serial port baud rate to %d failed (%s)\n", baud_rate, strerror(errno));
		exit(1);
  }

  if (flock(global_serial_fd, LOCK_EX | LOCK_NB) == -1)
  {
    fprintf(stderr, "Error: obtaining exclusive access to %s failed (%s)\n", serial_port, strerror(errno));
  }
}

int
main(int argc, char **argv)
{
  // NOTE(Ryan): For non-root access, user must be part of 'dialout' group
  initialise_serial_port("/dev/ttyACM0", B57600, 1);

  disable_logging();

  MemArena *perm_arena = mem_arena_allocate(GB(1));

  global_serial_output_len = KB(1); 
  global_serial_output.str = MEM_ARENA_PUSH_ARRAY(perm_arena, u8, global_serial_output_len); 


	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_console),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;
}
