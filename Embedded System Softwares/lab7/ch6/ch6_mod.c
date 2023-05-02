#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define PIR 17
#define LED 5

static int irq_num;

struct my_timer_info {
	struct timer_list timer;
	long delay_jiffies;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t) {
	printk("ch6: Jiffies %ld\n", jiffies);
	gpio_set_value(LED, 0);
}

static irqreturn_t ch6_pir_isr(int irq, void *dev_id) {
	unsigned long flags;

	gpio_set_value(LED, 0);
	gpio_set_value(LED, 1);

	mod_timer(&my_timer.timer, jiffies + my_timer.delay_jiffies);
	
	return IRQ_HANDLED;
}

static int __init ch6_mod_init(void) {
	int ret = 0;
	printk("ch6: Init module\n");

	gpio_request_one(PIR, GPIOF_IN, "PIR");
	gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");
	irq_num = gpio_to_irq(PIR);

	ret = request_irq(irq_num, ch6_pir_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);

	if(ret) {
		printk("ch6: Unable to request IRQ: %d\n", irq_num);
		free_irq(irq_num, NULL);
	 } else printk("ch6: Set IRQ to %d\n", irq_num);
	
	my_timer.delay_jiffies = msecs_to_jiffies(2000);
	my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
	timer_setup(&my_timer.timer, my_timer_func, 0);
	return 0;
}

static void __exit ch6_mod_exit(void) {
	printk("ch6: Exit module\n");

	disable_irq(irq_num);
	free_irq(irq_num, NULL);
	gpio_set_value(LED, 0);

	gpio_free(PIR);
	gpio_free(LED);
	
	del_timer(&my_timer.timer);
}

module_init(ch6_mod_init);
module_exit(ch6_mod_exit);
