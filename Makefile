# Makefile for the reMarkable Printer Application
#
# Licensed under Apache License v2.0.  See the file "LICENSE" for more
# information.

prefix = $(DESTDIR)/usr
bindir = $(prefix)/bin
unitdir = $(DESTDIR)$(shell pkg-config --variable=systemdsystemunitdir systemd)
depflags = $(shell pkg-config --cflags --libs pappl)

all: remarkable-printer-app

remarkable-printer-app: remarkable-printer-app.c
	$(CC) -o $@ -g -Os -Wall $^ $(depflags)

clean:
	rm -f remarkable-printer-app

install: all
	mkdir -p $(bindir)
	cp remarkable-printer-app $(bindir)
	mkdir -p $(unitdir)
	cp remarkable-printer-app.service $(unitdir)
