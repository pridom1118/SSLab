#include <linux/init.h>
#include <linux/module.h>

static int my_id = -1;

int get_my_id(void) {
	return my_id;
}

int set_my_id(int id) {
	my_id = id;
	return my_id == id;
}

EXPORT_SYMBOL(get_my_id);
EXPORT_SYMBOL(set_my_id);

static int __init ch1_mod1_init(void) {
	printk(KERN_NOTICE "Init mod1\n");
}

static void __exit ch1_mod1_exit(void) {
	printk(KERN_NOTICE "Exit mod1\n");
}

module_init(ch1_mod1_init);
module_exit(ch1_mod1_exit);
