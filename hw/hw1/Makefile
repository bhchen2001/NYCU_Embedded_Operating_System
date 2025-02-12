CC = aarch64-linux-gnu-gcc

HW1_SRC = hw1.c
HW1_TARGET = hw1

DRIVER_SRC = driver.c
obj-m := $(DRIVER_SRC:.c=.o)

MAKEFILE = $(PWD)/linux
MAKEFILE_FLAG = ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

.PHONY: all clean

all: module_build $(HW1_TARGET)

module_build:
	sudo make $(MAKEFILE_FLAG) -C $(MAKEFILE) M=$(PWD) modules

$(HW1_TARGET): $(HW1_SRC)
	$(CC) -o $@ $^

clean:
	rm -f $(HW1_TARGET)
	sudo make $(MAKEFILE_FLAG) -C $(MAKEFILE) M=$(PWD) clean