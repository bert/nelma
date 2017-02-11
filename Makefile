PREFIX?= $(HOME)/software

all: nelma

install: nelma-install man-install

nelma:
	cd src && $(MAKE) PREFIX=$(PREFIX) all

nelma-install:
	cd src && $(MAKE) PREFIX=$(PREFIX) install

INSTALL_MAN?=	install

man-install:
	$(INSTALL_MAN) doc/nelma-cap.1 $(PREFIX)/man/man1
	$(INSTALL_MAN) doc/nelma-drc.1 $(PREFIX)/man/man1

doxygen:
	doxygen

clean:
	rm -rf doc/html
	cd src && $(MAKE) clean

.PHONY: all clean doxygen install man-install nelma nelma-install
