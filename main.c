#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

int transfer_to_serial(int in, int out)
{
	unsigned char c;
	int byte_count;

	byte_count = read(in, &c, sizeof(c));
	if (byte_count > 0)
		write(out, &c, byte_count);
	return byte_count;
}

int transfer_from_serial(int in, int out)
{
	unsigned char buffer[512];
	int byte_count;

	byte_count = read(in, buffer, sizeof(buffer));
	if (byte_count > 0)
		write(out, buffer, byte_count);
	return byte_count;
}

void configure_input(int in, struct termios *old_options)
{
	struct termios options;

	tcgetattr(in, &options);

	*old_options = options;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	options.c_cflag |= CLOCAL | CREAD | HUPCL;
	options.c_lflag = 0;

	tcflush(in, TCIFLUSH);
	tcsetattr(in, TCSANOW, &options);
}

void configure_terminal(int terminal, int baud)
{
	struct termios options;

	tcgetattr(terminal, &options);

	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~INLCR & ~ICRNL & ~INPCK;
	options.c_iflag |= IGNPAR;
	options.c_cflag &= ~PARENB & ~CSTOPB & ~CSIZE & ~CRTSCTS & ~IXANY;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	options.c_cflag |= CLOCAL | CREAD | HUPCL | CS8;
	options.c_lflag = 0;

	cfsetspeed(&options, baud);
	tcflush(terminal, TCIFLUSH);
	tcsetattr(terminal, TCSANOW, &options);
}

void unconfigure_input(int in, struct termios *old_options)
{
	tcsetattr(in, TCSANOW, old_options);
}

int main(int argc, char **argv)
{
	int in;
	int out;
	int terminal;
	int speed;
	struct termios old_input_options;
	struct timeval timeout;
	fd_set readfds;

	if (argc < 2) {
		printf("Please specify a terminal.\n");
		return 1;
	}
	if (argc == 3)
		speed = atoi(argv[2]);
	else
		speed = 115200;

	in = dup(0);
	out = dup(1);
	terminal = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);

	configure_terminal(terminal, speed);
	configure_input(in, &old_input_options);

	for (;;) {
		FD_ZERO(&readfds);
		FD_SET(terminal, &readfds);
		FD_SET(in, &readfds);

		timeout.tv_sec  = 0;
		timeout.tv_usec = 500000;

		if (select(FD_SETSIZE, &readfds, NULL, NULL, &timeout) == -1) {
			break;
		} else {
			if (FD_ISSET(terminal, &readfds) && transfer_from_serial(terminal, out) == 0)
				break;

			if (FD_ISSET(in, &readfds) && transfer_to_serial(in, terminal) == 0)
				break;
		}
	}

	unconfigure_input(in, &old_input_options);

	close(terminal);
	close(in);
	close(out);

	return 0;
}

