// SPDX-License-Identifier: zlib-acknowledgement

#include "base-inc.h"

#include "test-inc.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/file.h>
#include <sys/ioctl.h>


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
execute_serial_cmd(String8 input, u32 min_wait_time, u32 max_wait_time)
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

#define SERIAL_PORT "/dev/ttyACM0"
#define BAUD_RATE B57600

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

  MemArena *perm_arena = mem_arena_allocate(GB(1));

  global_serial_output_len = KB(1); 
  global_serial_output.str = MEM_ARENA_PUSH_ARRAY(perm_arena, u8, global_serial_output_len); 

  
  // execute_cmd(str, min_wait_time, max_wait_time);

  String8 s = s8_lit("test\n");
  writex(global_serial_fd, s.str, s.size); 

  // -1 to account for NULL terminator byte included
  u32 bytes_expected = sizeof("Test\n") - 1;
  u32 bytes_available = 0;
  while (bytes_available != bytes_expected)
  {
    ioctl(global_serial_fd, FIONREAD, &bytes_available);
  }
  
	size_t bytes_read = readx(global_serial_fd, 
                            global_serial_output.str, 
                            bytes_expected);
  global_serial_output.size = bytes_read;

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
