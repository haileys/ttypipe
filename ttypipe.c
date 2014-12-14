#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define ESCAPE 28 // ctrl+backslash

static bool interactive = false;

struct termios orig_termios, raw_termios;

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
    }
}

static void
raw_on()
{
    if(interactive) {
        if(tcsetattr(0, TCSAFLUSH, &raw_termios) < 0) {
            perror("raw_on");
            exit(EXIT_FAILURE);
        }
    }
}

static void
raw_off()
{
    if(interactive) {
        if(tcsetattr(0, TCSAFLUSH, &orig_termios) < 0) {
            perror("raw_off");
            exit(EXIT_FAILURE);
        }
    }
}

static void
fatal(const char* msg)
{
    raw_off();
    perror(msg);
    exit(EXIT_FAILURE);
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
    raw_on();

    while(true) {
        char c;
        int rc = read(0, &c, 1);

        if(rc < 0) {
            if(errno == EINTR) {
                continue;
            }

            fatal("read");
        }

        if(rc == 0) {
            break;
        }

        if(interactive && c == ESCAPE) {
            printf(":");
            fflush(stdout);

            while(1) {
                rc = read(0, &c, 1);

                if(rc < 0) {
                    if(errno == EINTR) {
                        continue;
                    }

                    fatal("read");
                }

                if(rc == 0) {
                    goto quit;
                }

                if(c == '\\' || c == ESCAPE) {
                    // pass through ^\ to tty
                    printf("\b \b");
                    fflush(stdout);
                    c = ESCAPE;
                    break;
                }

                if(c == 'q' || c == 'Q') {
                    // quit
                    printf("\b \b");
                    fflush(stdout);
                    goto quit;
                }

                printf("\a");
            }
        }

        if(ioctl(tty_fd, TIOCSTI, &c) < 0) {
            if(errno == EIO) {
                // re-open the terminal
                close(tty_fd);

                tty_fd = open(argv[1], O_RDWR);

                if(tty_fd < 0) {
                    fatal("open");
                }

                // try pushing the character again, and bail out if it fails
                // the second time
                if(ioctl(tty_fd, TIOCSTI, &c) == 0) {
                    continue;
                }
            }

            fatal("ioctl(TIOCSTI)");
        }
    }

quit:
    if(interactive) {
        raw_off();
    }

    return 0;
}
