# Makefile for B-tree implementation
CC = gcc
CFLAGS = -Wall -g -std=c99
SRCS = main.c btree.c
OBJS = $(SRCS:.c=.o)
TARGET = btree

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
