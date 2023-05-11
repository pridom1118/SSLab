#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>

struct my_list {
	struct list_head list;
	int id;
};

struct my_list M;

static int __init simple_linked_list_init(void) {
	struct my_list *tmp = 0;
	struct list_head *pos = 0;
	unsigned int i;

	printk("simple linked list: INIT MODULE\n");

	INIT_LIST_HEAD(&M.list);

	for(i = 0; i < 5; ++i) {
		tmp = (struct my_list*) kmalloc(sizeof(struct my_list), GFP_KERNEL);
		tmp->id = i;
		printk("simple linked list: Insert [%d]\n", tmp->id);
		list_add(&tmp->list, &M.list);
	}

	i = 0;

	printk("simple linked list: use list_for_each & list_entry\n");

	list_for_each(pos, &M.list) {
		tmp = list_entry(pos, struct my_list, list); //name of the list_head field in the structure
		printk("simple linked list: pos[%d], id[%d]\n", i, tmp->id);
		++i;
	}

	i = 0;

	printk("simple linked list: use list_for_each_entry\n");
	list_for_each_entry(tmp, &M.list, list) {
		printk("simple linked list: pos[%d], id[%d]\n", i, tmp->id);
		++i;
	}
	return 0;
}

static void __exit simple_linked_list_exit(void) {
	struct my_list *tmp = 0;
	struct list_head *pos = 0;
	struct list_head *q = 0;
	unsigned int i = 0;
	
	printk("simple linked list: EXIT MODULE\n");

	list_for_each_safe(pos, q, &M.list) {
		tmp = list_entry(pos, struct my_list, list);
		printk("simple linked list: free pos[%d], id[%d]\n", i, tmp->id);
		kfree(tmp);
		++i;
	}
}

module_init(simple_linked_list_init);
module_exit(simple_linked_list_exit);
