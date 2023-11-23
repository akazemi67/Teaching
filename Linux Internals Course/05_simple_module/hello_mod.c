#include <linux/module.h>   // included for all kernel modules
#include <linux/kernel.h>   // included for KERN_INFO
#include <linux/init.h>     // included for __init and __exit macros

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abolfazl Kazemi");
MODULE_DESCRIPTION("Hello World Module!");

static char *hellstr = "CharParam";
static int hellarray[5] = {1, 2, 3, 4, 5};
static int arr_argc = 0;

module_param(hellstr, charp, 0);
MODULE_PARM_DESC(hellstr, "A character string");

module_param_array(hellarray, int, &arr_argc, 0);
MODULE_PARM_DESC(hellarray, "An array of integers");


static int __init hello_init(void) {
    int i;
    printk(KERN_INFO "Hello World Loaded!\n");
    pr_info("Str param is: %s\n", hellstr);

    for (i = 0; i < ARRAY_SIZE(hellarray); i++)
        pr_info("array[%d] = %d\n", i, hellarray[i]);
    pr_info("Got %d arguments for int array.\n", arr_argc);

    return 0;           //Nonzero return means that the module couldn't be loaded.
}


/* --------------- Executing Privileged Instruction ------------ */
static void check_interrupt_flag(void){
    long rflags = 0;
    asm("pushf; pop %0;" : "=a" (rflags));
    if( (1<<9) & rflags )
        printk(KERN_INFO "Interrupt-IS-Enabled\n");
    else
        printk(KERN_INFO "Interrupt-is-Disabled\n");
}

static void __exit hello_cleanup(void) {
    printk(KERN_INFO "Good Bye HelloMod!\n");
    
    printk(KERN_INFO "DISabling ints...\n");
    asm("cli");
    check_interrupt_flag();

    printk(KERN_INFO "ENabling ints...\n");
    asm("sti");
    check_interrupt_flag();
}

module_init(hello_init);
module_exit(hello_cleanup);


