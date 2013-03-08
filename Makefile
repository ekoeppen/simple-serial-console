CFLAGS = -O2
SRCS = main.c
PROG = serial

all: serial

clean:
	rm -f $(PROG)

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $+
