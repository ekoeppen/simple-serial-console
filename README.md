simple-serial-console
=====================

Just run `./build.sh`. Then run `./serial TTY_DEVICE` (e.g. `./serial /dev/ttyS8`).

Make sure set the baud rate on the TTY with `stty -F TTY_DEVICE BAUD_RATE` first! (e.g. `stty -F /dev/ttyS8 115200`)

At the moment you _cannot_ exit the program from the terminal it was run in. You have to kill the process from somewhere else. Let me know if you have a better solution.

Tested on Cygwin.

Refactored from http://www.plunk.org/~grantham/cgi-bin/blog.cgi?id=00015.