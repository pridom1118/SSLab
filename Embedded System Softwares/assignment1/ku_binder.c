#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "ku_binder.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("201811244");

#define DEV_NAME "ku_binder_dev"

/* Character Device Driver */
static dev_t dev_num;
static struct cdev *cd_cdev;

/* Wait queues */
wait_queue_head_t my_wqueue[KBINDER_SNUM_MAX];

/* Service Related Variables */
static int service_num;
static char service_name[KBINDER_SNUM_MAX][KBINDER_NAME_MAX];

/* RPC lists */
struct rpc_list {
	struct list_head list;
	struct binder_msg rpc;
};

static struct rpc_list rpc_list_head[KBINDER_SNUM_MAX]; // stores the requests from the client
spinlock_t list_lock[KBINDER_SNUM_MAX]; // locks for each lists

static int ku_binder_open(struct inode *inode, struct file *file) {
	return 0;
}

static int ku_binder_release(struct inode *inode, struct file *file) {
	return 0;
}

static int ku_binder_register(char* sname) {
	int ret, i;
	char kern_buf[KBINDER_NAME_MAX];
	ret = copy_from_user(kern_buf, sname, KBINDER_NAME_MAX);
    
	if(ret == -EFAULT) {
		printk("ku_binder: copy_from_user failed at register\n");
		return -1;
	}

	if(service_num < KBINDER_SNUM_MAX) {
		strlcpy(service_name[service_num], kern_buf, KBINDER_NAME_MAX);
		for(i = 0; i < service_num; ++i) {
			if(strncmp(service_name[i], kern_buf, KBINDER_NAME_MAX) == 0) {
				printk("ku_binder: duplicate service\n");
				return -1;
			}
		}
		return service_num++;
	} else {
		printk("cannot register %d: %d\n", service_num, KBINDER_SNUM_MAX);
		return -1;
	}		
}

static int ku_binder_query(char* sname) {
	int ret, i;
	char kern_buf[KBINDER_NAME_MAX];
	
	ret = copy_from_user(kern_buf, sname, KBINDER_NAME_MAX);

	if(ret == -EFAULT) {
		printk("ku_binder: copy_from_user failed at query\n");
		return -1;
	}

	ret = -1;

	for(i = 0; i < service_num; ++i) {
		if(strncmp(kern_buf, service_name[i], KBINDER_NAME_MAX) == 0) ret = i;
	}
	return ret;
}

static int ku_binder_rpc(struct binder_msg *user_msg) {
	int ret = -1, snum = -1;

	struct rpc_list *tmp;

	tmp = (struct rpc_list*)kmalloc(sizeof(struct rpc_list), GFP_KERNEL);

	/* acquire the service number */
	ret = copy_from_user(&tmp->rpc, user_msg, sizeof(struct binder_msg));
	printk("ku_binder: got snum: %d fcode: %d param: %d\n", tmp->rpc.snum, tmp->rpc.fcode, tmp->rpc.param.user_vol.vol);
	
	/* FAILED TO COPY FROM USER */
	if(ret == -EFAULT) {
		ret = -1;
	} else {
		snum = tmp->rpc.snum;
		spin_lock(&list_lock[snum]);
		list_add_tail(&tmp->list, &rpc_list_head[snum].list);
		printk("ku_binder: added snum: %d fcode: %d param: %d\n", tmp->rpc.snum, tmp->rpc.fcode, tmp->rpc.param.user_vol.vol);	
		wake_up_interruptible(&my_wqueue[snum]);
		spin_unlock(&list_lock[snum]);
		ret = 0;
	}

	return ret;
}

static int ku_binder_read(struct binder_msg *user_msg) {
	int ret = -1, snum = -1;

	struct rpc_list *tmp = 0;
	struct binder_msg server_msg;

	/* acquire the service number first */
	ret = copy_from_user(&server_msg, user_msg, sizeof(struct binder_msg));
	snum = server_msg.snum;
	printk("ku_binder: got snum %d\n", snum);

	if(ret == -EFAULT) {
		printk("ku_binder: copy_from_user() failed at read\n");
		return -1;
	}

	/* sleep the process if the list is empty */
	wait_event_interruptible_exclusive(my_wqueue[snum], !list_empty(&rpc_list_head[snum].list)); 

	/* pass the request to the server */
	spin_lock(&list_lock[snum]);
	tmp = list_first_entry(&rpc_list_head[snum].list, struct rpc_list, list);
	ret = copy_to_user(user_msg, &tmp->rpc, sizeof(struct binder_msg));
	printk("ku_binder: got snum: %d with fcode %d with vol %d\n", tmp->rpc.snum, tmp->rpc.fcode, tmp->rpc.param.user_vol.vol);
	list_del(&tmp->list);
	kfree(tmp);
	spin_unlock(&list_lock[snum]);

	return ret;
}

static long ku_binder_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int ret, i;
	char kern_buf[KBINDER_NAME_MAX];

	switch(cmd) {
		case BINDER_REG:
			ret = ku_binder_register((char*) arg);
			break;
		case BINDER_QUERY:
			ret = ku_binder_query((char*) arg);
			break;
		case BINDER_RPC:
			ret = ku_binder_rpc((struct binder_msg*) arg);
			break;
		case BINDER_READ:
			ret = ku_binder_read((struct binder_msg*) arg);
			break;
		default:
			return -1;
	}
	return ret;
}

struct file_operations ku_binder_fops = {
	.unlocked_ioctl = ku_binder_ioctl,
	.open = ku_binder_open,
	.release = ku_binder_release
};

static int __init ku_binder_init(void) {
	int i;
	
	printk("ku_binder: Init Module\n");

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &ku_binder_fops);
	cdev_add(cd_cdev, dev_num, 1);
	service_num = 0;

	/* Initialization */
	for(i = 0; i < KBINDER_SNUM_MAX; ++i) {
		init_waitqueue_head(&my_wqueue[i]);
		INIT_LIST_HEAD(&rpc_list_head[i].list);
		spin_lock_init(&list_lock[i]);
	}

	return 0;
}

static void __exit ku_binder_exit(void) {
	struct rpc_list *tmp = 0;
	struct list_head *pos = 0, *q = 0;
	int i = 0, j = 0;

	printk("ku_binder: Exit Module\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);

	for(i = 0; i < KBINDER_SNUM_MAX; ++i) {
		j = 0;
	
		list_for_each_safe(pos, q, &rpc_list_head[i].list) {
			tmp = list_entry(pos, struct rpc_list, list);
			list_del(pos);
			kfree(pos);
			++j;
		}
	}
}

module_init(ku_binder_init);
module_exit(ku_binder_exit);
