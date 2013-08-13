#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <getopt.h>

#define ESCAPE_CHAR 0x1d
#define EXIT_CHAR '.'
#define RESET_CHAR 'r'

#define RESET_SEQUENCE "\033c"

static int in;
static int out;
static int terminal;
static struct termios old_input_options;
static int escape_state;
static int speed;
static char *terminal_device;
static int opt_ascii = 0;
static int opt_reset = 0;
static int opt_translate = 0;
static char *options = "art";

bool transfer_to_terminal(void)
{
	unsigned char c;
	bool r = true;
	int byte_count;

	byte_count = read(in, &c, sizeof(c));
	if (byte_count > 0) {
		if (!escape_state) {
			if (c != ESCAPE_CHAR) {
				if (!opt_translate || c != '\n') {
					write(terminal, &c, byte_count);
				} else {
					write(terminal, "\r\n", 2);
				}
			} else {
				escape_state = 1;
			}
		} else {
			switch (c) {
			case EXIT_CHAR:
				r = false;
				break;
			case RESET_CHAR:
				write(out, RESET_SEQUENCE,
				      sizeof(RESET_SEQUENCE));
				break;
			default:
				write(terminal, &c, byte_count);
			}
			escape_state = 0;
		}
	} else {
		r = false;
	}
	return r;
}

bool transfer_from_terminal(void)
{
	unsigned char buffer[512];
	int i;
	int byte_count;
	bool r = true;

	byte_count = read(terminal, buffer, sizeof(buffer));
	if (byte_count > 0) {
		if (opt_ascii) {
			for (i = 0; i < byte_count; i++) {
				if (buffer[i] > 128 ||
				    (buffer[i] < ' ' &&
				     buffer[i] != '\t' && buffer[i] != '\r' && buffer[i] != '\n'))
					buffer[i] = '.';
			}
		}
		write(out, buffer, byte_count);
	} else {
		r = false;
	}
	return r;
}

void configure_input(void)
{
	struct termios options;

	tcgetattr(in, &options);

	old_input_options = options;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	options.c_cflag |= CLOCAL | CREAD | HUPCL;
	options.c_lflag = 0;

	tcflush(in, TCIFLUSH);
	tcsetattr(in, TCSANOW, &options);
}

void configure_terminal(int baud)
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

void unconfigure_input(void)
{
	tcsetattr(in, TCSANOW, &old_input_options);
}

void signal_handler(int signum)
{
	switch (signum) {
	case SIGQUIT:
	case SIGINT:
	case SIGTERM:
		unconfigure_input();
		break;
	default:
		break;
	}
}

void setup_signal_handlers(void)
{
	signal(SIGQUIT, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
}

void handle_cmd_line(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'a':
			opt_ascii = 1;
			break;
		case 'r':
			opt_reset = 1;
			break;
		case 't':
			opt_translate = 1;
			break;
		case '?':
			exit(1);
			break;
		default:
			break;
		}
	}
	if (optind < argc) {
		terminal_device = argv[optind];
		optind++;
	} else {
		printf("Please specify a terminal.\n");
		exit(1);
	}
	if (optind < argc)
		speed = atoi(argv[optind]);
	else
		speed = 115200;

}

int main(int argc, char **argv)
{
	struct timeval timeout;
	fd_set readfds;

	handle_cmd_line(argc, argv);

	in = dup(0);
	out = dup(1);
	terminal = open(terminal_device, O_RDWR | O_NOCTTY | O_NONBLOCK);

	configure_terminal(speed);
	configure_input();

	setup_signal_handlers();
	escape_state = 0;
	for (;;) {
		FD_ZERO(&readfds);
		FD_SET(terminal, &readfds);
		FD_SET(in, &readfds);

		timeout.tv_sec  = 0;
		timeout.tv_usec = 500000;

		if (select(FD_SETSIZE, &readfds, NULL, NULL, &timeout) == -1) {
			break;
		} else {
			if (FD_ISSET(terminal, &readfds) &&
			    transfer_from_terminal() == false)
				break;

			if (FD_ISSET(in, &readfds) &&
			    transfer_to_terminal() == false)
				break;
		}
	}

	unconfigure_input();
	if (opt_reset)
		write(out, RESET_SEQUENCE, sizeof(RESET_SEQUENCE));

	close(terminal);
	close(in);
	close(out);

	return 0;
}

