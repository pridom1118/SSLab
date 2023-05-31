#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "simple_sysfs"

static dev_t dev_num;
static struct cdev *cd_cdev;
static struct class *cl;

char value[1024];

static int simple_sysfs_open(struct inode *inode, struct file *file) {
	printk("simple_sysfs: simple_open\n");
	return 0;
}

ssize_t simple_sysfs_write(struct file *file, const char* ubuf, size_t count, loff_t *f_pos) {
	printk("simple_sysfs: simple_write\n");
	return 0;
}

ssize_t simple_sysfs_read(struct file *file, char* ubuf, size_t count, loff_t *f_pos) {
	printk("simple_sysfs: simple_read\n");
	return 0;
}

static int simple_sysfs_release(struct inode *inode, struct file *file) {
	printk("simple_sysfs: simple_release\n");
	return 0;
}

static struct file_operations simple_sysfs_fops = {
	.open = simple_sysfs_open,
	.write = simple_sysfs_write,
	.read = simple_sysfs_read,
	.release = simple_sysfs_release
};

static ssize_t simple_show_value(struct device *dev, struct device_attribute *attr, char *buf) {
	return snprintf(buf, PAGE_SIZE, "%s\n", value);
}

static ssize_t simple_store_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	sscanf(buf, "%s", value);
	return strnlen(buf, PAGE_SIZE);
}

static DEVICE_ATTR(value, 0664, simple_show_value, simple_store_value);

static int __init simple_sysfs_init(void) {
	int err = 0;
	struct device *dev_ret;

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &simple_sysfs_fops);
	cdev_add(cd_cdev, dev_num, 1);

	/* create class */
	if(IS_ERR(cl = class_create(THIS_MODULE, DEV_NAME))) {
		goto class_err;
	}

	/* create sysfs directory */
	if(IS_ERR(dev_ret = device_create(cl, NULL, dev_num, "%s", DEV_NAME))) {
		goto device_err;
	}

	/* create sysfs file */

	if ((err = device_create_file(dev_ret, &dev_attr_value))) {
		goto device_file_err;
	}

	return 0;

device_file_err:
	device_destroy(cl, dev_num);
device_err:
	class_destroy(cl);
class_err:
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
	return -1;
}

static void __exit simple_sysfs_exit(void) {
	device_destroy(cl, dev_num);
	class_destroy(cl);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(simple_sysfs_init);
module_exit(simple_sysfs_exit);
