#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abolfazl Kazemi");
MODULE_DESCRIPTION("Importer Module!");

extern void print_hello(int);
extern void add_two_numbers(int, int);
extern int MAH_GLOBAL_VAR;

/*
 * The function has been written just to call the functions which are in other module. 
 * This way you can also write modules which does provide some functionality to the other modules.
 */
static int __init im_init(void) {
    printk(KERN_INFO "Hello from Importer Module.\n");
    print_hello(5);
    add_two_numbers(11, 28);
    printk(KERN_INFO "Value of MAH_GLOBAL_VAR %d\n", MAH_GLOBAL_VAR);
    return 0;
}

static void __exit im_exit(void) {
    printk(KERN_INFO "Bye from Importer Module.\n");
}

module_init(im_init);
module_exit(im_exit);

