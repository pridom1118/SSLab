#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

#define SWITCH 12
#define LED 5

MODULE_LICENSE("GPL");
MODULE_AUTHOR("201811244");

struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
};

static struct my_timer_info my_timer;
static int ret;

static void my_timer_func(struct timer_list *t) {
	struct my_timer_info *info = from_timer(info, t, timer);

	ret = gpio_get_value(SWITCH);
	if(ret) {
		gpio_set_value(LED, 1);
		printk("ch5_201811244: Jiffies: %ld, pressed\n", jiffies);
	} else {
		gpio_set_value(LED, 0);
		printk("ch5_201811244: Jiffies: %ld, not pressed\n", jiffies);
	}
	mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
}

static int __init ch5_init(void) {
	printk("ch5_201811244: init module\n");
	gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");
	gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

	my_timer.delay_jiffies = msecs_to_jiffies(500); // 0.5 seconds (500ms)
	timer_setup(&my_timer.timer, my_timer_func, 0);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
	add_timer(&my_timer.timer);

	return 0;
}

static void __exit ch5_exit(void) {
	printk("ch5_201811244: exit module\n");
	gpio_set_value(LED, 0);
	del_timer(&my_timer.timer);

	gpio_free(SWITCH);
	gpio_free(LED);
}

module_init(ch5_init);
module_exit(ch5_exit);
