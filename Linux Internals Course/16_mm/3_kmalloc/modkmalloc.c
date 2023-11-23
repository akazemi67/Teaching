#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

static void *ptr;
unsigned int alloc_size = 1024;

unsigned int loop  = 8192;
module_param(loop, uint, 0);

static int test_hello_init(void) {
	int i;
	for(i=1; i < loop; i++) {
		ptr = kmalloc(alloc_size*i, GFP_KERNEL);
		if(!ptr) {
			/* handle error */
			pr_err("memory allocation failed\n");
			return -ENOMEM;
		} else {
			pr_info("Memory allocated of size:%uKB successfully:%px\n", i, ptr);
			kfree(ptr);
			pr_info("Memory freed\n");
		}
	}
	return -1;
}

static void test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);


