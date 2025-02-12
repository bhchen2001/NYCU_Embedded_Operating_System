/***************************************************************************
**
* \file led_driver.c
* \details Simple GPIO driver explanation
* \author EmbeTronicX
* \Tested with Linux raspberrypi 5.4.51-v7l+
******************************************************************************
*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* 
 * Device Constants
 *     - LED_COUNT: Number of LEDs in the array
 *     - SEGMENT_COUNT: Number of segments in the 7-segment display
 *     - DEVICE_COUNT: Number of devices (LED & 7-segment)
 */
#define LED_COUNT 8
#define SEGMENT_COUNT 7
#define DEVICE_COUNT 2
#define DEVICE_NAME_PREFIX "display_"

/* 
 * GPIO pin definitions
 */
static const int led_pins[LED_COUNT] = {2, 3, 4, 17, 27, 22, 10, 9};
static const int segment_pins[SEGMENT_COUNT] = {14, 15, 18, 23, 24, 25, 8};

/*
 * Device structure
 */
struct display_dev {
    dev_t dev_minor_num;
    struct class *dev_class;
    struct cdev cdev;
    char name[20];
    const int *pins;
    int pin_count;
};

static struct display_dev devices[DEVICE_COUNT];

/*
 * 7-segment display patterns for digits 0-9
 */
static const unsigned char segment_patterns[10] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};

static int display_open(struct inode *inode, struct file *file);
static int display_release(struct inode *inode, struct file *file);
static ssize_t display_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t display_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = display_read,
    .write = display_write,
    .open = display_open,
    .release = display_release,
};

static int display_open(struct inode *inode, struct file *file) {
    struct display_dev *dev = container_of(inode->i_cdev, struct display_dev, cdev);
    file->private_data = dev;
    pr_info("%s device opened\n", dev->name);
    return 0;
}

static int display_release(struct inode *inode, struct file *file) {
    struct display_dev *dev = file->private_data;
    pr_info("%s device closed\n", dev->name);
    return 0;
}

/*
 * Read function
 *     - Reads the current state of the GPIO pins
 */
static ssize_t display_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    struct display_dev *dev = filp->private_data;
    uint8_t gpio_state[32] = {0};
    int i;

    for (i = 0; i < dev->pin_count; i++) {
        gpio_state[i] = gpio_get_value(dev->pins[i]);
    }

    if (copy_to_user(buf, gpio_state, dev->pin_count)) {
        return -EFAULT;
    }

    return dev->pin_count;
}

/*
 * Write function
 *     - Get a digit and set the GPIO pins according to the device type
 *     - For '\n', clear the display
 */
static ssize_t display_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    struct display_dev *dev = filp->private_data;
    char value;
    int i;

    if (len != 1) {
        return -EINVAL;
    }

    if (copy_from_user(&value, buf, 1)) {
        return -EFAULT;
    }

    if (value == '\n') {
        // Clear the display
        for (i = 0; i < dev->pin_count; i++) {
            gpio_set_value(dev->pins[i], 0);
        }
    }
    else if (value < '0' || value > '9') {
        return -EINVAL;
    }

    if (strcmp(dev->name, "led") == 0) {
        // LED handling - display binary pattern
        int num = value - '0';
        for (i = 0; i < dev->pin_count; i++) {
            gpio_set_value(dev->pins[i], (num > i));
        }
    } else {
        // 7-segment handling - display digit
        int digit = value - '0';
        for (i = 0; i < dev->pin_count; i++) {
            gpio_set_value(dev->pins[i], (segment_patterns[digit] >> i) & 1);
        }
    }

    return len;
}

/*
 * Initialize GPIO pins
 */
static int init_gpio_pins(const int *pins, int count, const char *label) {
    int i;

    for (i = 0; i < count; i++) {
        if (!gpio_is_valid(pins[i])) {
            pr_err("GPIO %d is not valid\n", pins[i]);
            goto gpio_cleanup;
        }

        if (gpio_request(pins[i], label)) {
            pr_err("GPIO %d request failed\n", pins[i]);
            goto gpio_cleanup;
        }

        gpio_direction_output(pins[i], 0);
        gpio_export(pins[i], false);
    }

    return 0;

gpio_cleanup:
    for (i--; i >= 0; i--) {
        gpio_unexport(pins[i]);
        gpio_free(pins[i]);
    }
    return -EINVAL;
}

static int __init display_driver_init(void) {
    int i;
    dev_t dev_base;

    if((alloc_chrdev_region(&dev_base, 0, DEVICE_COUNT, "display_dev")) <0){
        pr_err("Failed to allocate device numbers\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev_base), MINOR(dev_base));

    // initialize LED device
    devices[0].dev_minor_num = MKDEV(MAJOR(dev_base), 0);
    strcpy(devices[0].name, "led");
    devices[0].pins = led_pins;
    devices[0].pin_count = LED_COUNT;

    // initialize 7-segment device
    devices[1].dev_minor_num = MKDEV(MAJOR(dev_base), 1);
    strcpy(devices[1].name, "7seg");
    devices[1].pins = segment_pins;
    devices[1].pin_count = SEGMENT_COUNT;

    // creating struct class
    devices[0].dev_class = class_create(THIS_MODULE, "display_class");
    if (devices[0].dev_class == NULL) {
        pr_err("Failed to create struct class\n");
        goto r_class;
    }

    // initialize both devices
    for (i = 0; i < DEVICE_COUNT; i++) {
        cdev_init(&devices[i].cdev, &fops);
        if(cdev_add(&devices[i].cdev, devices[i].dev_minor_num, 1) < 0){
            pr_err("Failed to add cdev for %s\n", devices[i].name);
            goto cleanup;
        }

        if (device_create(devices[0].dev_class, NULL, devices[i].dev_minor_num, NULL,
                          DEVICE_NAME_PREFIX "%s", devices[i].name) == NULL) {
            pr_err("Failed to create device for %s\n", devices[i].name);
            goto cleanup;
        }

        if(init_gpio_pins(devices[i].pins, devices[i].pin_count, devices[i].name) < 0){
            goto cleanup;
        }
    }

    pr_info("Display driver initialized successfully\n");
    return 0;

cleanup:
    for (i--; i >= 0; i--) {
        cdev_del(&devices[i].cdev);
        device_destroy(devices[0].dev_class, devices[i].dev_minor_num);
    }

r_class:
    class_destroy(devices[0].dev_class);

r_unreg:
    unregister_chrdev_region(devices[0].dev_minor_num, DEVICE_COUNT);

    return -EINVAL;
}

static void __exit display_driver_exit(void) {
    int i, j;

    for (i = 0; i < DEVICE_COUNT; i++) {
        for (j = 0; j < devices[i].pin_count; j++) {
            gpio_unexport(devices[i].pins[j]);
            gpio_free(devices[i].pins[j]);
        }
        cdev_del(&devices[i].cdev);
        device_destroy(devices[0].dev_class, devices[i].dev_minor_num);
    }

    class_destroy(devices[0].dev_class);
    unregister_chrdev_region(devices[0].dev_minor_num, DEVICE_COUNT);
    pr_info("Display driver removed successfully\n");
}

module_init(display_driver_init);
module_exit(display_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Dual Display Driver for LED Array and 7-Segment Display");