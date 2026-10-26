# CS631 APUE Group Project: sws - Simple Web Server
# Integrated Build Configuration

CC ?= gcc
CFLAGS ?= -Wall -Werror -Wextra -Wformat=2 \
	  -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow \
	  -std=c99 -D_GNU_SOURCE -O2
LDFLAGS ?= -lmagic

SRCS = main.c network.c daemon.c http.c header.c response.c \
       uri.c dir_index.c mime.c cgi.c logger.c
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
