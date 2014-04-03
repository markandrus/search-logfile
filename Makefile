SRCS=$(shell ls *.c)
OBJS=$(SRCS:.c=.o)
BIN=main
CFLAGS=-std=c99 -Wall -Wextra -Weverything -O2

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

.PHONY: all clean test

all: $(BIN)

test: all
	time ./$(BIN) zebra /usr/share/dict/words

clean:
	rm -f $(OBJS) $(BIN)
