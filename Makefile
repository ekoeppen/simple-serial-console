CFLAGS = -O2 -Wall
SRCS = main.c
PROG = serial

all: serial

clean:
	rm -f $(PROG)

$(PROG): $(SRCS) Makefile
	$(CC) $(CFLAGS) -o $@ $(SRCS)
