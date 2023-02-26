// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "test-inc.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

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

INTERNAL char *
execute_serial_cmd(String8 input)
{
	writex(global_serial_fd, input.str, input.size);

	// sleep(5);
	size_t bytes_read = readx(global_serial_fd, 
                            global_serial_output.str, 
                            global_serial_output_max_len);
  global_serial_output[bytes_read] = '\0';
  global_serial_output.size = bytes_read;

  return (char *)global_serial_output.str;
}

INTERNAL void
test_console(void **state)
{
  assert_string_equal(execute_serial_cmd(s8_lit("test\n")), "Test\n");
}

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B57600

int
main(int argc, char **argv)
{
	int serial_fd = open(SERIAL_PORT, O_RDWR | O_NDELAY | O_NOCTTY);
	if (serial_fd < 0) 
  {
    fprintf(stderr, "Error: opening serial port " SERIAL_PORT " failed (%s)\n", strerror(errno));
		return -1;
	}

	struct termios serial_options = {0};
	tcgetattr(serial_fd, &serial_options);
	
	serial_options.c_cflag = BAUD_RATE | CS8 | CLOCAL | CREAD;
	serial_options.c_iflag = IGNPAR;

	tcflush(serial_fd, TCIFLUSH);
	if (tcsetattr(serial_fd, TCSANOW, &serial_options) == -1)
  {
    fprintf(stderr, "Error: setting serial port baud rate to " STRINGIFY(BAUD_RATE) " failed (%s)\n", strerror(errno));
		return -1;
  }

  MemArena *perm_arena = mem_arena_allocate(GB(1));

  String8 serial_output = {0};
  serial_output.str = MEM_ARENA_PUSH_ARRAY(perm_arena, u8, KB(1)); 

  // TODO(Ryan): must check amount of bytes read as say after 5 seconds want 10 bytes, 8 bytes might be available
  // i.e. read reads UP TO count bytes

	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_console),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;

  // TODO(Ryan): Could do pattern checking on output to account for logging etc.
  // In this case, have to make read non-blocking or on a timeout to account for unknown buffer size?
}

  // select is fine for low-traffic?
  /*
  fd_set set;
  FD_ZERO(&set);
  FD_SET(serial_fd, &set);

  struct timeval timeout = {0};
  timeout.tv_sec = 1;

  rv = select(filedesc + 1, &set, NULL, NULL, &timeout);
  if(rv == -1)
    perror("select"); // an error accured 
  else if(rv == 0)
    printf("timeout"); // a timeout occured
  else
    read( filedesc, buff, len ); // there was data to read
  close(filedesc);
  */
