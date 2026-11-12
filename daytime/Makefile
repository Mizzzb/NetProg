# Makefile для клиента daytime (UDP)

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -O2 -std=c11
TARGET  = daytime_client
SRC     = daytime_client.c
OBJ     = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJ)
