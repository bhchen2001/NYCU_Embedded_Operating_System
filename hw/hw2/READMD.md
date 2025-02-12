# NYCU Embedded OS HW2

## Instruction

### Environment Setting

* Host Machine
    * Ubuntu 22.04.1
    * Linux Kernel Version: `6.8.0-45-generic`
    * Cross Compiler: `aarch64-linux-gnu-gcc 11.4.0`
* Target Machine
    * Raspberry Pi 3B+
    * Linux Kernel Version: `6.1.93-v8+`

### Submission Files

```
312551074_陳柏翰_hw2/
├── hw2.c
├── hw2_demo.mp4
├── hw2.h
└── Makefile
```

### Building

* In this homework, I build the program on host machine instead of the Rpi 3B+ then transfer the binary executable to the target machine.
* Run the following command to build the program

```bash
make
```

### Transfer and Run

* Transfer the binary image to the target machine

```bash
scp ./hw2  <target_ip>
```

* On Rpi, execute the `hw2` program

```bash
./hw2 <port_num>
```

* On the host machine, execute the `hw2_checker` program

```bash
./hw2_checker <target_ip> <port_num>
```