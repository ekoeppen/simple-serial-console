CFLAGS = -O2 -Wall -std=c99 -D_BSD_SOURCE
SRCS = main.c
PROG = serial

all: serial

clean:
	rm -f $(PROG)

$(PROG): $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)
