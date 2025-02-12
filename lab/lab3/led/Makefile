DRIVER_SRC = led_driver.c
obj-m := $(DRIVER_SRC:.c=.o)

WRITER_SRC = writer.c
WRITER_TARGET = writer

BUILD_PATH = $(PWD)/../linux

.PHONY: all clean

all: module_build $(WRITER_TARGET)

module_build:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(BUILD_PATH) M=$(PWD) modules

$(WRITER_TARGET): $(WRITER_SRC)
	aarch64-linux-gnu-gcc -o $@ $^

clean:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(BUILD_PATH) M=$(PWD) clean
	rm -f $(WRITER_TARGET)