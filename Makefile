INSTALL_PREFIX ?= /usr/local
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99

all: ttypipe

.PHONY: install uninstall

install: ttypipe
	cp ttypipe $(INSTALL_PREFIX)/bin/ttypipe

uninstall:
	rm $(INSTALL_PREFIX)/bin/ttypipe
