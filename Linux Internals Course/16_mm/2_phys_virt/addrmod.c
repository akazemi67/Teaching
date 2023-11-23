#include <linux/module.h> 
#include <linux/printk.h> 
#include <asm/io.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
char *str;

int init_module(void) {
    str = kmalloc(11*sizeof(char), GFP_ATOMIC);
    if(!str){
        pr_err("Cannot allocate memory!\n");
        return -1;
    }   
    strncpy(str, "AABBCCDDEE", 10); 
    str[10] = 0;
   
    pr_info("Content: %s\n", str);
	pr_info("Virt Addr(hashed): 0x%p \n", str);
    pr_info("Virt Addr: 0x%px \n", str);
	
	//It is only valid to use this function on addresses directly mapped or allocated via kmalloc. 
    pr_info("Phys Addr: 0x%llx \n",(unsigned long long)virt_to_phys((void*)str));

    return 0; 
} 
 
void cleanup_module(void) { 
    if(str){
        pr_info("Str before release: %s\n", str);
        kfree(str);
    }
    pr_info("Goodbye.\n"); 
} 



