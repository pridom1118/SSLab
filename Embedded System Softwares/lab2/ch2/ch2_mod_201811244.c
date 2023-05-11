#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("201811244");

#define DEV_NAME "ch2_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define CH_IOCTL_NUM 'z'
#define GET_IOCTL _IOWR(CH_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define SET_IOCTL _IOWR(CH_IOCTL_NUM, IOCTL_NUM2, unsigned long)
#define ADD_IOCTL _IOWR(CH_IOCTL_NUM, IOCTL_NUM3, unsigned long)
#define MUL_IOCTL _IOWR(CH_IOCTL_NUM, IOCTL_NUM4, unsigned long)

static unsigned long res = 0;

static int ch2_ioctl_open(struct inode *inode, struct file *file) {
	printk("ch2: open\n");
	return 0;
}

static int ch2_ioctl_release(struct inode *inode, struct file *file) {
	printk("ch2: release\n");
	return 0;
}

static long ch2_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case GET_IOCTL:
			printk("ch2: current res : %ld\n", res);
			return (long)res;
		case SET_IOCTL:
			printk("ch2: change res from %ld to %ld\n", res, arg);
			res = arg;
			break;
		case ADD_IOCTL:
			printk("ch2: res += %ld\n", arg);
			res += arg;
			return (long)res;
		case MUL_IOCTL:
			printk("ch2: res *= %ld\n", arg);
			res *= arg;
			return (long)res;
		default:
			return -1;
	}
	return 0;
}

struct file_operations ch2_ioctl_fops = {
	.open = ch2_ioctl_open,
	.release = ch2_ioctl_release,
	.unlocked_ioctl = ch2_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch2_ioctl_init(void) {
	printk("ch2: init module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME); 
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ch2_ioctl_fops);
	cdev_add(cd_cdev, dev_num, 1);
	return 0;
}

static void __exit ch2_ioctl_exit(void) {
	printk("ch2: exit module\n");

	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ch2_ioctl_init);
module_exit(ch2_ioctl_exit);
