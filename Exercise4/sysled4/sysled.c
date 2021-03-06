#include <linux/gpio.h> 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#define MAJORNUM 65
#define MINORNUM 0

#define GPIO_NUM 163

MODULE_LICENSE("Dual BSD/GPL");

static struct cdev my_cdev;
struct file_operations my_fops;

static int sysled_init(void) {
	int errNo;
	int devno = MKDEV(MAJORNUM, MINORNUM);

	errNo = gpio_request(GPIO_NUM, "gpio164");
	if (errNo < 0) {
		printk(KERN_ALERT "gpio_request failed: %d", errNo);
		return -1;
	}

	gpio_direction_output(GPIO_NUM, 1);
	cdev_init(&my_cdev, &my_fops);
	
	errNo = register_chrdev_region(devno, 1, "sysled4");
	if (errNo < 0) {
		printk(KERN_ALERT "Registration of device number failed! %d\n", errNo);
		gpio_free(GPIO_NUM);
	}

	if (cdev_add(&my_cdev, devno, 1) < 0) {
		printk(KERN_ALERT "Cdev_add failed! :(");
		unregister_chrdev_region(my_cdev.dev, 1);
		gpio_free(GPIO_NUM);
	}
	
	return 0;
}

static void sysled_exit(void) {
	cdev_del(&my_cdev);
	unregister_chrdev_region(my_cdev.dev, 1);
	gpio_free(GPIO_NUM);
}

int sysled_open(struct inode *inode, struct file * fp) {
	int major, minor;
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Opening Sys_led device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

int sysled_release(struct inode * inode, struct file * fp) {
	int major, minor;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
	printk("Closing/releasing Sys_led device [major], [minor]: %i, %i\n", major, minor);

	return 0;
}

ssize_t sysled_read(struct file * fp, char __user * buf, size_t count, loff_t * fpos) {
	int gpioValue = gpio_get_value(GPIO_NUM);
	unsigned long err;

	if (gpioValue == 0) {
		err = copy_to_user(buf, "0", 2);
	}
	else {
		err = copy_to_user(buf, "1", 2);
	}

	if (err != 0) {
		printk(KERN_ALERT "Copy to user gave an error..!\n");
	}

	return 2;
}

ssize_t sysled_write(struct file * fp, const char __user * ubuf, size_t count, loff_t * fpos) {
	int value;
	if (sscanf(ubuf, "%d", &value) != 1) {
		printk(KERN_ALERT "Error reading");
	}

	gpio_set_value(GPIO_NUM, value);

	return count;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = sysled_read,
	.open = sysled_open,
	.write = sysled_write,
	.release = sysled_release,
};

module_init(sysled_init);
module_exit(sysled_exit);

