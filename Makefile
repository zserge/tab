CFLAGS ?= -Wall -Werror -pedantic -std=c89

PREFIX = /usr/local

all: tab

tab: tab.c
	$(CC) $(CFLAGS) tab.c -o tab

install: tab
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f tab "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/tab"
	@echo "Installed to $(DESTDIR)$(PREFIX)/bin/tab"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/tab"
	@echo "Uninstalled from $(DESTDIR)$(PREFIX)/bin/tab"

clean:
	rm -f tab tab.exe

.PHONY: all clean install uninstall
