# ttypipe

Remote control for your TTYs.

ttypipe pipes any data it receives from stdin straight through to a TTY's input buffer (by way of the `TIOCSTI` ioctl).

For convenience, ttypipe will also switch the controlling terminal into raw mode to receive input if it's running under a TTY itself.

### Installation and usage

```
# make install
# ttypipe /dev/tty1
```
