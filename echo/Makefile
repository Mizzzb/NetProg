# Makefile для клиента echo (TCP)

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -O2 -std=c11
TARGET  = echo_client
SRC     = echo_client.c
OBJ     = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ)
