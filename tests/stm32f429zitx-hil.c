// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "test-inc.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/file.h>

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B57600

GLOBAL int global_serial_fd;
GLOBAL String8 global_serial_output;
GLOBAL size_t global_serial_output_len;


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
                            global_serial_output_len);
  global_serial_output.str[bytes_read] = '\0';
  global_serial_output.size = bytes_read;

  return (char *)global_serial_output.str;
}

INTERNAL void
test_console(void **state)
{
  assert_string_equal(execute_serial_cmd(s8_lit("test\n")), "Test\n");
}


int
main(int argc, char **argv)
{
  // normal user to 'dialout' group
  
  // IMPORTANT(Ryan): If specifying non-blocking arguments in here can affect VTIM
  // and result in 'resource temporarily unavaiable'
	global_serial_fd = open(SERIAL_PORT, O_RDWR);
	if (global_serial_fd < 0) 
  {
    fprintf(stderr, "Error: opening serial port " SERIAL_PORT " failed (%s)\n", strerror(errno));
		return -1;
	}

	struct termios serial_options = {0};
	if (tcgetattr(global_serial_fd, &serial_options) == -1)
  {
    fprintf(stderr, "Error: obtaining original serial port settings failed (%s)\n", strerror(errno));
		return -1;
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
  serial_options.c_cc[VTIME] = (10 * 10);    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  serial_options.c_cc[VMIN] = 0;

  cfsetspeed(&serial_options, BAUD_RATE);

	if (tcsetattr(global_serial_fd, TCSANOW, &serial_options) == -1)
  {
    fprintf(stderr, "Error: setting serial port baud rate to " STRINGIFY(BAUD_RATE) " failed (%s)\n", strerror(errno));
		return -1;
  }

  if (flock(global_serial_fd, LOCK_EX | LOCK_NB) == -1)
  {
    fprintf(stderr, "Error: obtaining exclusive access to " SERIAL_PORT " failed (%s)\n", strerror(errno));
		return -1;
  }

  // See if there are bytes available to read
  // int bytes;
  // ioctl(fd, FIONREAD, &bytes);

  MemArena *perm_arena = mem_arena_allocate(GB(1));

  global_serial_output_len = KB(1); 
  global_serial_output.str = MEM_ARENA_PUSH_ARRAY(perm_arena, u8, global_serial_output_len); 

  printf("Waiting for input\n");

	size_t bytes_read = readx(global_serial_fd, 
                            global_serial_output.str, 
                            5);

  printf("Recieved: %.*s\n", s8_varg(global_serial_output));

  // TODO(Ryan): must check amount of bytes read as say after 5 seconds want 10 bytes, 8 bytes might be available
  // i.e. read reads UP TO count bytes

#if 0
	struct CMUnitTest tests[] = {
    cmocka_unit_test(test_console),
  };

  int cmocka_res = cmocka_run_group_tests(tests, NULL, NULL);

  return cmocka_res;
#else
  return 0;
#endif

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
