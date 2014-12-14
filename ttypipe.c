#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static bool sigint_occurred = false;

static bool interactive = false;

struct termios orig_termios, raw_termios;

static void
sigint()
{
    sigint_occurred = true;
}

static void
init_termios()
{
    interactive = isatty(0);

    if(interactive) {
        if(tcgetattr(0, &orig_termios) < 0) {
            perror("tcgetattr");
            exit(EXIT_FAILURE);
        }

        raw_termios = orig_termios;
        cfmakeraw(&raw_termios);

        if(tcsetattr(0, TCSAFLUSH, &raw_termios) < 0) {
            perror("tcsetattr(raw_termios)");

            if(tcsetattr(0, TCSAFLUSH, &orig_termios) < 0) {
                perror("tcsetattr(orig_termios)");
            }

            exit(EXIT_FAILURE);
        }
    }
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

    init_termios();

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

    if(interactive) {
        if(tcsetattr(0, TCSAFLUSH, &orig_termios) < 0) {
            perror("tcsetattr(orig_termios)");
            return 1;
        }
    }

    return 0;
}
