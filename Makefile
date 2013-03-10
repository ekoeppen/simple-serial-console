CFLAGS = -O2 -Wall -std=c99
SRCS = main.c
PROG = serial

all: serial

clean:
	rm -f $(PROG)

$(PROG): $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)
