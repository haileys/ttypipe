#define _DEFAULT_SOURCE

#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

static bool sigint_occurred = false;

static void
sigint()
{
    sigint_occurred = true;
}

int
main(int argc, const char** argv)
{
    if(argc <= 1) {
        fprintf(stderr, "usage: %s <tty>\n", argv[0]);
        return 1;
    }

    int tty_fd = open(argv[1], O_RDWR);

    if(tty_fd < 0) {
        perror("open");
        return 1;
    }

    struct termios orig_termios;

    if(isatty(0)) {
        if(tcgetattr(0, &orig_termios) < 0) {
            perror("tcgetattr");
            return 1;
        }

        struct termios raw_termios = orig_termios;
        cfmakeraw(&raw_termios);

        if(tcsetattr(0, TCSAFLUSH, &raw_termios) < 0) {
            perror("tcsetattr(raw_termios)");

            if(tcsetattr(0, TCSAFLUSH, &orig_termios) < 0) {
                perror("tcsetattr(orig_termios)");
            }

            return 1;
        }
    }

    signal(SIGINT, sigint);

    while(!sigint_occurred) {
        char c;
        int rc = read(0, &c, 1);

        if(rc < 0) {
            perror("read");
            break;
        }

        if(rc == 0) {
            break;
        }

        if(ioctl(tty_fd, TIOCSTI, &c) < 0) {
            if(errno != EIO) {
                perror("ioctl(TIOCSTI)");
            }
            break;
        }
    }

    if(isatty(0)) {
        if(tcsetattr(0, TCSAFLUSH, &orig_termios) < 0) {
            perror("tcsetattr(orig_termios)");
            return 1;
        }
    }
    
    return 0;
}
