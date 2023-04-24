#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

struct int_list {
	struct list_head list;
	int id;
};

struct int_list my_list;

static int __init llex_init(void) {
	struct int_list *tmp = 0;
	struct list_head *pos = 0;
	unsigned int i;

	printk("llex: init\n");

	INIT_LIST_HEAD(&my_list.list);

	for(i = 0; i < 5; ++i) {
		tmp = (struct int_list*) kmalloc(sizeof(struct int_list), GFP_KERNEL);
		tmp->id = i + 1;
		list_add_tail(&tmp->list, &my_list.list);
	}

	tmp = list_first_entry(&my_list.list, struct int_list, list);
	printk("first entry: %d\n", tmp->id);
	return 0;
}

static void __exit llex_exit(void) {
	struct int_list *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	unsigned int i = 0;

	printk("llex: exit\n");

	list_for_each_safe(pos, q, &my_list.list) {
		tmp = list_entry(pos, struct int_list, list);
		printk("llex: free llex[%d] = %d\n", i, tmp->id);
		list_del(pos);
		kfree(tmp);
		++i;
	}
}

module_init(llex_init);
module_exit(llex_exit);
