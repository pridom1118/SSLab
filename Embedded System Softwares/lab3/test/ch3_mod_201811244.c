#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/delay.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("201811244");

#define DEV_NAME "ch3_dev"
#include "ch3.h"

struct msg_list {
	struct list_head list;
	struct msg_st msg;
};

static struct msg_list msg_list_head;
spinlock_t msg_list_lock;

void delay(int sec) {
	int i, j;
	for(j = 0; j < sec; ++j) {
		for(i = 0; i < 1000; ++i) {
			udelay(1000);
		}
	}
}

static int ch3_open(struct inode *inode, struct file *file) {
	printk("ch3_201811244: OPEN\n");
	return 0;
}

static int ch3_release(struct inode *inode, struct file *file) {
	printk("ch3_201811244: RELEASE\n");
	return 0;
}

static int ch3_msg_write(struct msg_st *buf) {
	struct msg_list *tmp;
	int ret;

	tmp = (struct msg_list*) kmalloc(sizeof(struct msg_list), GFP_KERNEL);
	spin_lock(&msg_list_lock);
	ret = copy_from_user(&tmp->msg, buf, sizeof(struct msg_st));
	list_add_tail(&tmp->list, &msg_list_head.list);
	spin_unlock(&msg_list_lock);

	printk("ch3_201811244: new message added!\n");
	return ret;
}

static int ch3_msg_read(struct msg_st *buf) {
	struct msg_list *tmp = 0;
	struct msg_st empty;
	int ret;

	empty.len = 0;
	memset(empty.str, 0, sizeof(empty.str));
	empty.str[0] = '0';

	delay(5);
	spin_lock(&msg_list_lock);
	if(list_empty(&msg_list_head.list)) {
		ret = copy_to_user(buf, &empty, sizeof(struct msg_st));
	} else {
		tmp = list_first_entry(&msg_list_head.list, struct msg_list, list);
		ret = copy_to_user(buf, &tmp->msg, sizeof(struct msg_st));
		list_del(&tmp->list);
		kfree(tmp);
	}
	spin_unlock(&msg_list_lock);
	
	return ret;

}

static long ch3_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	struct msg_st *user_buf;
	int ret;

	user_buf = (struct msg_st*) arg;

	switch(cmd) {
		case CH3_IOCTL_READ:
			ret = ch3_msg_read(user_buf);
			return (long) ret;
		case CH3_IOCTL_WRITE:
			ret = ch3_msg_write(user_buf);
			return (long) ret;
		default:
			return -1;
	}
	return 0;
}

struct file_operations ch3_fops = {
	.open = ch3_open,
	.release = ch3_release,
	.unlocked_ioctl = ch3_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch3_init(void) {
	int ret;
	printk("ch3_201811244: INIT\n");

	INIT_LIST_HEAD(&msg_list_head.list);
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ch3_fops);
	ret = cdev_add(cd_cdev, dev_num, 1);

	if(ret < 0) {
		printk("ch3_201811244: FAILED TO ADD CHRDEV\n");
		return -1;
	}
	return 0;
}

static void __exit ch3_exit(void) {
	struct msg_list *tmp = 0;
	struct list_head *pos = 0, *q = 0;
	unsigned int i = 0;

	printk("ch3_201811244: EXIT\n");

	list_for_each_safe(pos, q, &msg_list_head.list) {
		tmp = list_entry(pos, struct msg_list, list);
		list_del(pos);
		kfree(tmp);
		++i;
	}
	
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(ch3_init);
module_exit(ch3_exit);
