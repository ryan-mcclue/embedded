// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

INTERNAL void
writex(int fd, void *buf, size_t count)
{
  int bytes_written = write(fd, buf, count);
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

INTERNAL void
readx(int fd, void *buf, size_t count)
{
  int bytes_read = read(fd, buf, count);
  if (bytes_read == -1)
  {
    fprintf(stderr, "Error: read failed (%s)\n", strerror(errno));
    exit(1);
  }
}

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B57600

int
main(int argc, char **argv)
{
	int serial_fd = open(SERIAL_PORT, O_RDWR | O_NDELAY | O_NOCTTY);
	if (serial_fd < 0) 
  {
    fprintf(stderr, "Error: opening serial port " SERIAL_PORT " failed (%s)\n", sterrorr(errno));
		return -1;
	}

	struct termios serial_options = {0};
	tcgetattr(serial_fd, &serial_options);
	
	serial_options.c_cflag = BAUD_RATE | CS8 | CLOCAL | CREAD;
	serial_options.c_iflag = IGNPAR;

	tcflush(serial_fd, TCIFLUSH);
	tcsetattr(serial_fd, TCSANOW, &serial_options);

  String8 serial_output = {0};
  serial_output.str = malloc();

  String8 test_console_input = s8_lit("test");
  String8 test_console_output_expected = s8_lit("Test");
	writex(serial_fd, test_console_input.str, test_console_input.size);

	// sleep(5);
	readx(serial_fd, serial_output.str, test_console_output_expected.size);
  serial_output.size = test_console_output_expected.size;

  // TODO(Ryan): Could do pattern checking on output to account for logging etc.
  // In this case, have to make read non-blocking or on a timeout to account for unknown buffer size?

  //size_t find_index = s8_match(serial_output, test_console_output_expected, 0);
  //if (find_index != serial_output.size)  
  //{
  //  puts("passed");
  //}

	close(serial_fd);

  return 0;
}
