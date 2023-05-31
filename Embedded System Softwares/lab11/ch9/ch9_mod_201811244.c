#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("201811244");

#define BUF_SIZE 50

static struct proc_dir_entry *ent;
static char str[BUF_SIZE] = "empty";
static int count;

static int proc_info_show(struct seq_file *seq, void *v) {
	seq_printf(seq, "%d\n", count);
	seq_printf(seq, "%s\n", str);
	return 0;
}

static int proc_info_open(struct inode *inode, struct file *file) {
	return single_open(file, proc_info_show, NULL);
}

static ssize_t proc_info_write(struct file *file, const char __user *ubuf, size_t ubuf_len, loff_t *pos) {
	int len;
	char buf[BUF_SIZE] = { 0, };

	if(ubuf_len > BUF_SIZE) return -1;

	if(copy_from_user(buf, ubuf, ubuf_len)) return -1;

	memset(str, 0, sizeof(str));

	len = strlen(buf);
	strlcpy(str, buf, len);

	return len;
}

static ssize_t proc_info_read(struct file *file, char __user *ubuf, size_t ubuf_len, loff_t *pos) {
	int i, len = 0;
	
	count = 0;
	len = strlen(str); 
	
	for(i = 0; i < len; i++) {
		if(str[i] == ' ') count++;
	}
	
	printk("str: %s\n", str);
	printk("count: %d\n", count);
	return 0;
}

static const struct file_operations proc_info_fops = {
	.open = proc_info_open,
	.read = proc_info_read,
	.write = proc_info_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init simple_proc_init(void) {
	ent = proc_create("ch9_proc", 0666, NULL, &proc_info_fops);
	return 0;
}

static void __exit simple_proc_exit(void) {
	proc_remove(ent);
}

module_init(simple_proc_init);
module_exit(simple_proc_exit);
