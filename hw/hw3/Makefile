# CC = aarch64-linux-gnu-gcc
CC = gcc

HW3_SRC = hw3.c
HW3_TARGET = hw3

.PHONY: all test clean

all: $(HW3_TARGET)
test: $(TEST_CLIENT_TARGET)

$(HW3_TARGET): $(HW3_SRC)
	$(CC) -o $@ $^

clean:
	rm -f $(HW3_TARGET)