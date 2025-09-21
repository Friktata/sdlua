CC = gcc
CFLAGS = -std=c99 -Iinclude -g -lSDL3 -lSDL3_image -lSDL3_ttf -llua -ldl -lpthread -lm #-Wall -Wextra

SRC_DIR = src
INC_DIR = include
BUILD_DIR = .
BUILD_NAME = sdlua

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

TARGET = $(BUILD_DIR)/$(BUILD_NAME)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o

.PHONY: all clean
