CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = myls

SRC = src/main.c \
      src/ls.c \
      src/options.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o myls myls.exe

rebuild: clean all

.PHONY: all clean rebuild