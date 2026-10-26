# CS631 APUE Group Project: sws - Simple Web Server
# Member 1: Core Network & Daemon Engine

CC ?= gcc
CFLAGS ?= -Wall -Werror -Wextra -Wformat=2 \
	  -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow \
	  -std=c99 -D_GNU_SOURCE -O2
LDFLAGS ?=

SRCS = main.c network.c daemon.c
OBJS = $(SRCS:.c=.o)
TARGET = sws

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
