# NYCU Embedded OS HW1

## Instruction

### Environment Setting

* Host Machine
    * Ubuntu 22.04.1
    * Linux Kernel Version: `6.8.0-45-generic`
    * Cross Compiler: `aarch64-linux-gnu-gcc 11.4.0`
    * Linux Kernel for Building: `rpi-6.1.y` (same as Lab2)
* Target Machine
    * Raspberry Pi 3B+
    * Linux Kernel Version: `6.1.93-v8+`

### Submission Files

```
312551074_陳柏翰_HW1/
├── driver.c
├── hw1.c
├── Makefile
└── writer.h
```

### Building

* In this homework, I build the program on host machine instead of the Rpi 3B+ then transfer the binary image to the target machine.
* Before building the program, need to put the linux kernel source code under the same directory with `Makefile`
* The version of linux kernel source code is `rpi-6.1.y` (same as lab2)

```
312551074_陳柏翰_HW1/
├── driver.c
├── hw1.c
├── Makefile
├── writer.h
└── linux/
```

* Then, run the following command to build the program

```bash
make
```

### Transfer and Run

* Transfer the binary image to the target machine

```bash
scp ./hw1 ./driver.ko <target_ip>
```

* On the target machine, insert the kernel module and run the program

```bash
sudo insmod driver.ko
sudo ./hw1
```