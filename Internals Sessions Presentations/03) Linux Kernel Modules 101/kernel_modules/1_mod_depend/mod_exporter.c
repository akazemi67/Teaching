#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abolfazl Kazemi");
MODULE_DESCRIPTION("Exporter Module!");

int MAH_GLOBAL_VAR = 1337;
EXPORT_SYMBOL(MAH_GLOBAL_VAR);

void print_hello(int num) {
	while (num--) {
		printk(KERN_INFO "Just Saying Hello!!!\n");
	}
}
EXPORT_SYMBOL(print_hello);

void add_two_numbers(int a, int b) {
	printk(KERN_INFO "Sum of the numbers %d\n", a + b);
}
EXPORT_SYMBOL(add_two_numbers);

static int __init ex_init(void) {
	printk(KERN_INFO "Hello from Exporter Module.\n");
	return 0;
}

static void __exit ex_exit(void) {
	printk(KERN_INFO "Bye from Exporter Module.\n");
}

module_init(ex_init);
module_exit(ex_exit);

