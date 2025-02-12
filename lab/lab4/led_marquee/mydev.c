#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
MODULE_LICENSE("GPL");

#define KERNEL_BUFFER_SIZE 1024

struct my_device_data {
    struct cdev cdev;
    char kernel_buffer[KERNEL_BUFFER_SIZE];
};
static struct my_device_data *mydev_data;

static const unsigned short seg_for_c[27] = {
    0b1111001100010001, // A
    0b0000011100000101, // b
    0b1100111100000000, // C
    0b0000011001000101, // d
    0b1000011100000001, // E
    0b1000001100000001, // F
    0b1001111100010000, // G
    0b0011001100010001, // H
    0b1100110001000100, // I
    0b1100010001000100, // J
    0b0000000001101100, // K
    0b0000111100000000, // L
    0b0011001110100000, // M
    0b0011001110001000, // N
    0b1111111100000000, // O
    0b1000001101000001, // P
    0b0111000001010000, // q
    0b1110001100011001, // R
    0b1101110100010001, // S
    0b1100000001000100, // T
    0b0011111100000000, // U
    0b0000001100100010, // V
    0b0011001100001010, // W
    0b0000000010101010, // X
    0b0000000010100100, // Y
    0b1100110000100010, // Z
    0b0000000000000000  // Empty
};

static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "call my_open\n");
    
    struct my_device_data *data = container_of(inode->i_cdev, struct my_device_data, cdev);

    file->private_data = data;

    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "call my_release\n");

    // null the private_data
    file->private_data = NULL;

    return 0;
}

static ssize_t my_write(struct file *fp, const char __user *buf, size_t count, loff_t *fpos) {
    printk(KERN_INFO "call my_write\n");
    
    struct my_device_data *data = (struct my_device_data *)fp->private_data;
    ssize_t bytes_to_write = min(KERNEL_BUFFER_SIZE - *fpos, count);

    printk("fpos: %lld\n", *fpos);

    if (count > KERNEL_BUFFER_SIZE) {
        printk(KERN_ERR "count is too large\n");
        return -ENOMEM;
    }

    if (copy_from_user(data->kernel_buffer + *fpos, buf, bytes_to_write)) {
        printk(KERN_ERR "copy_from_user failed\n");
        return -EFAULT;
    }

    // update the write offset
    *fpos += bytes_to_write;

    return bytes_to_write;
}

static ssize_t my_read(struct file *fp, char __user *buf, size_t count, loff_t *fpos) {
    printk(KERN_INFO "call my_read\n");

    struct my_device_data *data = (struct my_device_data *)fp->private_data;
    char pattern[16] = {0};
    char c = data->kernel_buffer[(int)*fpos];
    unsigned short seg = 0;

    // assume that the count will always be 16
    if (count != 16) {
        printk(KERN_ERR "count is invalid\n");
        return -EINVAL;
    }

    if (c >= 'A' && c <= 'Z') {
        seg = seg_for_c[c - 'A'];
    }
    else if (c >= 'a' && c <= 'z') {
        seg = seg_for_c[c - 'a'];
    }
    else {
        printk(KERN_ERR "invalid character\n");
        return -EINVAL;
    }

    // turn unsigned short to char array
    for (int i = 0; i < 16; i++) {
        pattern[15 - i] = (seg & (1 << i)) ? '1' : '0';
    }

    // copy the pattern to user space
    if (copy_to_user(buf, pattern, 16)) {
        printk(KERN_ERR "copy_to_user failed\n");
        return -EFAULT;
    }

    // read one char every time
    *fpos += 1;

    return count;
}

static struct file_operations mydev_fops = {
    .open = my_open,
    .release = my_release,
    .write = my_write,
    .read = my_read
};

#define MAJOR_NUM 255
#define DEVICE_NAME "mydev"
#define NUM_DEVICES 1

static int mydev_init(void) {
    printk(KERN_INFO "call mydev_init\n");
    
    int ret;
    dev_t dev;

    // alloc the self-defined data structure for device
    mydev_data = kzalloc(sizeof(struct my_device_data), GFP_KERNEL);
    if (!mydev_data) {
        printk(KERN_ERR "kzalloc failed\n");
        return -ENOMEM;
    }

    // register the device
    dev = MKDEV(MAJOR_NUM, 0);
    ret = register_chrdev_region(dev, NUM_DEVICES, DEVICE_NAME);
    if (ret < 0) {
        kfree(mydev_data);
        printk(KERN_ERR "register_chrdev_region() failed\n");
        return ret;
    }

    // initialize the cdev
    cdev_init(&mydev_data->cdev, &mydev_fops);
    mydev_data->cdev.owner = THIS_MODULE;

    // add the cdev to the system
    ret = cdev_add(&mydev_data->cdev, dev, NUM_DEVICES);
    if (ret < 0) {
        unregister_chrdev_region(dev, NUM_DEVICES);
        kfree(mydev_data);
        printk(KERN_ERR "cdev_add() failed\n");
        return ret;
    }

    printk(KERN_INFO "My device is started with major number %d\n", MAJOR_NUM);
    return 0;
}

static void mydev_exit(void) {
    printk(KERN_INFO "call mydev_exit\n");
    
    dev_t dev = MKDEV(MAJOR_NUM, 0);

    // remove the cdev from the system
    unregister_chrdev_region(dev, NUM_DEVICES);
    cdev_del(&mydev_data->cdev);

    // free the self-defined data structure
    kfree(mydev_data);

    printk(KERN_INFO "my_exit success\n");
}

module_init(mydev_init);
module_exit(mydev_exit);