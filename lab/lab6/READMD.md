# NYCU Embedded OS Lab6

## Instruction

### Environment Setting

* Host Machine
    * Ubuntu 24.04.1
    * Linux Kernel Version: `6.8.0-45-generic`

### Submission Files

```
312551074_eos_lab6/
├── client.c
├── demo.sh
├── lab6.mp4
├── Makefile
└── server.c
```

### Demo

* In this lab, I build and run the program on the host machine instead of Rpi
* With the `demo.sh` script, the following error message will be shown with `tmux split-window -p` command in my host machine

```bash
size missing
```

* In my demo video, I change the command to

```bash
tmux split-window -h -l 60
tmux split-window -v -l 75
tmux split-window -v -l 66
tmux split-window -v -l 50
```

* Then run the `demo.sh` script

```bash
./demo.sh
```