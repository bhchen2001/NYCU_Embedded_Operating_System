CC = aarch64-linux-gnu-gcc

HW2_SRC = hw2.c
HW2_TARGET = hw2

.PHONY: all test clean

all: $(HW2_TARGET)
test: $(TEST_CLIENT_TARGET)

$(HW2_TARGET): $(HW2_SRC)
	$(CC) -o $@ $^

clean:
	rm -f $(HW2_TARGET)