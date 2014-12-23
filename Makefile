SRCS=$(shell ls *.c)
OBJS=$(SRCS:.c=.o)
BIN=main
CFLAGS=-std=c99 -Wall -Wextra -O2 $(shell pkg-config --cflags libpcre)
LDFLAGS=-lc $(shell pkg-config --libs libpcre) -g

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

.PHONY: all clean test

all: $(BIN)

test: all
	time ./$(BIN) zebra /usr/share/dict/words

test2: all
	time ./$(BIN) "[Tue Jan 07 14:02:02 2014]" /var/log/apache2/error_log

clean:
	rm -f $(OBJS) $(BIN)
