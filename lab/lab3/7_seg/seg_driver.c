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
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h> //GPIO

// pins is connected to these GPIOs
#define GPIO_COUNT 7
static const int gpio_pins[GPIO_COUNT] = {14, 15, 18, 23, 24, 25, 8};

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, 
                const char *buf, size_t len, loff_t * off);
static ssize_t my_etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
/******************************************************/

//File operation structure 
static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = etx_read,
    .write          = my_etx_write,
    .open           = etx_open,
    .release        = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
    uint8_t gpio_state[GPIO_COUNT] = {0};
    int i;

    //reading GPIO values
    for (i = 0; i < GPIO_COUNT; i++) {
        gpio_state[i] = gpio_get_value(gpio_pins[i]);
    }
    
    //write to user
    len = GPIO_COUNT;
    if( copy_to_user(buf, gpio_state, len) > 0) {
        pr_err("ERROR: Not all the bytes have been copied to user\n");
    }
    
    pr_info("Read function : GPIO states read\n");

    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
    uint8_t rec_buf[GPIO_COUNT] = {0};
    int i;

    if( copy_from_user( rec_buf, buf, len ) > 0) {
        pr_err("ERROR: Not all the bytes have been copied from user\n");
    }
    
    for (i = 0; i < GPIO_COUNT && i < len; i++) {
        if (rec_buf[i] == '1') {
            //set the GPIO value to HIGH
            gpio_set_value(gpio_pins[i], 1);
        } else if (rec_buf[i] == '0') {
            //set the GPIO value to LOW
            gpio_set_value(gpio_pins[i], 0);
        } else {
            pr_err("Unknown command for GPIO %d : Please provide either 1 or 0 \n", gpio_pins[i]);
        }
    }
    
    return len;
}

/* 
 *  my seg_driver write function
 *  user will write a single byte (representing a single digit) to the device file
 *  step
 *      1. get the digit from the user
 *      2. convert the digit to the state of the 7 gpio pins
 */

static ssize_t my_etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    char rec_buf, digit;
    int i;
    int ret;

    if (len != 1) {
        pr_err("Invalid data length\n");
        return -EINVAL;
    }

    ret = copy_from_user(&rec_buf, buf, len);
    if (ret < 0) {
        pr_err("copy_from_user() failed\n");
        return ret;
    }

    if (rec_buf < '0' || rec_buf > '9') {
        pr_err("Invalid digit\n");
        return -EINVAL;
    }

    /*
     * digit char represents the state of the 7 gpio pins
     * ''  -> state of gfedcba
     * '0' -> 0011111
     * '1' -> 0000110
     * '2' -> 1011011
     * '3' -> 1001111
     * '4' -> 1100110
     * '5' -> 1101101
     * '6' -> 1111101
     * '7' -> 0000111
     * '8' -> 1111111
     * '9' -> 1101111
     */
    switch (rec_buf) {
        case '0':
            digit = 0x3F;
            break;
        case '1':
            digit = 0x06;
            break;
        case '2':
            digit = 0x5B;
            break;
        case '3':
            digit = 0x4F;
            break;
        case '4':
            digit = 0x66;
            break;
        case '5':
            digit = 0x6D;
            break;
        case '6':
            digit = 0x7D;
            break;
        case '7':
            digit = 0x07;
            break;
        case '8':
            digit = 0x7F;
            break;
        case '9':
            digit = 0x6F;
            break;
        default:
            pr_err("Invalid digit\n");
            return -EINVAL;
    }

    for (i = 0; i < GPIO_COUNT; i++) {
        gpio_set_value(gpio_pins[i], (digit >> i) & 0x01);
    }

    return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    int i;

    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
        pr_err("Cannot allocate major number\n");
        goto r_unreg;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_del;
    }

    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
        pr_err( "Cannot create the Device \n");
        goto r_device;
    }

    //Checking the GPIOs are valid or not
    for (i = 0; i < GPIO_COUNT; i++) {
        if(gpio_is_valid(gpio_pins[i]) == false){
            pr_err("GPIO %d is not valid\n", gpio_pins[i]);
            goto r_device;
        }
        
        //Requesting the GPIO
        if(gpio_request(gpio_pins[i],"GPIO_LED") < 0){
            pr_err("ERROR: GPIO %d request\n", gpio_pins[i]);
            goto r_gpio;
        }
        
        //configure the GPIO as output
        gpio_direction_output(gpio_pins[i], 0);
        
        /* Using this call the GPIO 21 will be visible in /sys/class/gpio/
        ** Now you can change the gpio values by using below commands also.
        ** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
        ** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
        ** cat /sys/class/gpio/gpio21/value (read the value LED)
        ** 
        ** the second argument prevents the direction from being changed.
        */
        gpio_export(gpio_pins[i], false);
    }
    
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_gpio:
    for (; i >= 0; i--) {
        gpio_unexport(gpio_pins[i]);
        gpio_free(gpio_pins[i]);
    }
r_device:
    device_destroy(dev_class,dev);
r_class:
    class_destroy(dev_class);
r_del:
    cdev_del(&etx_cdev);
r_unreg:
    unregister_chrdev_region(dev,1);
    
    return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    int i;
    for (i = 0; i < GPIO_COUNT; i++) {
        gpio_unexport(gpio_pins[i]);
        gpio_free(gpio_pins[i]);
    }
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver for multiple LEDs");
MODULE_VERSION("1.33");