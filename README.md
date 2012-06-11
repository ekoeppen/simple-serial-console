simple-serial-console
=====================

Just run `./build.sh`. Then run `./serial TTY_DEVICE`. For example `./serial /dev/ttyS8`.

At the moment you _cannot_ exit the console from the terminal it was run in. You have to kill the process from somewhere else. Let me know if you have a better solution.

Tested on Cygwin.

Refactored from http://www.plunk.org/~grantham/cgi-bin/blog.cgi?id=00015.