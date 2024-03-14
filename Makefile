CFLAGS ?= -Wall -Werror -pedantic -std=c89

all:
	$(CC) $(CFLAGS) tabs.c -o tabs

clean:
	rm -f tabs tabs.exe
