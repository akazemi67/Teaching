#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static void *ptr;

static int test_hello_init(void) {
	ptr = kmalloc(1, GFP_KERNEL);
	printk("I got: %zu bytes of memory\n", ksize(ptr));
	kfree(ptr);
	
	ptr = kmalloc(20, GFP_KERNEL);
	printk("I got: %zu bytes of memory\n", ksize(ptr));
	kfree(ptr);
	
	ptr = kmalloc(3500, GFP_KERNEL);
	printk("I got: %zu bytes of memory\n", ksize(ptr));
	kfree(ptr);
	
	ptr = kmalloc(7900, GFP_KERNEL);
	printk("I got: %zu bytes of memory\n", ksize(ptr));
	kfree(ptr);
	
	return -1;
}

static void test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);

