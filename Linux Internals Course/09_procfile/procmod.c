#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> /* for copy_from_user */ 
#include <linux/version.h> 
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abolfazl Kazemi");
MODULE_DESCRIPTION("Procfs Module!");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0) 
    #define HAVE_PROC_OPS 
#endif 

#define PROCFS_MAX_SIZE 1024 
#define PROCFS_NAME "akazemi_proc" 
 
/* This structure hold information about the /proc file */ 
static struct proc_dir_entry *our_proc_file; 

/* This buffer is used for storing proc entry data  */ 
static char procfs_buffer[PROCFS_MAX_SIZE]; 
static unsigned long procfs_buffer_size = 0; 

void write_process_info(pid_t pid){
    struct task_struct *taskp;

    taskp = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if(taskp){
        sprintf(procfs_buffer, "pid:%d parent:%d comm:%s\n", taskp->pid, taskp->real_parent->pid, taskp->comm);
    }else{
        sprintf(procfs_buffer, "Process pid:%d not found!\n", pid);
    }
    procfs_buffer_size = strlen(procfs_buffer)+1;
}

/* This function is called then the /proc file is read */ 
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, 
                             size_t buffer_length, loff_t *offset) { 
    int len = procfs_buffer_size; 
    ssize_t ret = len; 

    if(*offset >= len)
        return 0;

    if (copy_to_user(buffer, procfs_buffer, len)) { 
        pr_info("copy_to_user failed\n"); 
        ret = 0; 
    } else { 
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name); 
        *offset += len; 
    }

    return ret; 
} 

/* This function is called when data is written to the /proc file. */ 
static ssize_t procfile_write(struct file *file, const char __user *buff, 
                              size_t len, loff_t *off) { 
    int i, pid;
    if(*off>=len)
        return *off;

    procfs_buffer_size = len; 
    if (procfs_buffer_size > PROCFS_MAX_SIZE) 
        procfs_buffer_size = PROCFS_MAX_SIZE; 

    if (copy_from_user(procfs_buffer, buff, procfs_buffer_size)) 
        return -EFAULT; 

    i = 0;
    while(procfs_buffer[i]>='0' && procfs_buffer[i]<='9')
        i++;
    procfs_buffer[i] = '\0'; 
    *off += procfs_buffer_size; 
    pr_info("procfile write %s\n", procfs_buffer); 

    if(!kstrtouint(procfs_buffer, 10, &pid)){
        pr_info("Getting process info for pid: %d\n", pid);
        write_process_info(pid);
    }else{
        pr_err("Error getting pid from user mode.\n");
    }

    return procfs_buffer_size; 
} 


#ifdef HAVE_PROC_OPS /* kernel>=5.6 */
static const struct proc_ops proc_file_fops = { 
    .proc_read = procfile_read, 
    .proc_write = procfile_write, 
}; 
#else 
static const struct file_operations proc_file_fops = { 
    .read = procfile_read, 
    .write = procfile_write, 
}; 
#endif 

static int __init procfs2_init(void) { 
    char initStr[32] = "Write PID to get Information!\n"; 
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops); 
    if (!our_proc_file) { 
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME); 
        return -ENOMEM; 
    } 

    strcpy(procfs_buffer, initStr);
    procfs_buffer_size = sizeof(initStr);

    pr_info("/proc/%s created\n", PROCFS_NAME); 
    return 0; 
} 

static void __exit procfs2_exit(void)  { 
    proc_remove(our_proc_file); 
    pr_info("/proc/%s removed\n", PROCFS_NAME); 
} 

module_init(procfs2_init); 
module_exit(procfs2_exit); 


