DRIVER_SRC = mydev.c
obj-m := $(DRIVER_SRC:.c=.o)

READER_SRC = reader.c
READER_TARGET = reader

WRITER_SRC = writer.c
WRITER_TARGET = writer

MAKEFILE_PATH = $(PWD)/linux

.PHONY: all module clean

all: x86_build $(READER_TARGET) $(WRITER_TARGET)

module: x86_build

x86_build: $(DRIVER_SRC)
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(MAKEFILE_PATH) M=$(PWD) modules

$(READER_TARGET): $(READER_SRC)
	aarch64-linux-gnu-gcc -o $@ $^

$(WRITER_TARGET): $(WRITER_SRC)
	aarch64-linux-gnu-gcc -o $@ $^

clean:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(MAKEFILE_PATH) M=$(PWD) clean
	rm -f $(READER_TARGET) $(WRITER_TARGET)
	