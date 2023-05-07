#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "../include/mah_char.h"

MODULE_DESCRIPTION("A sample character device");
MODULE_AUTHOR("Abolfazl Kazemi");
MODULE_LICENSE("GPL");

#define LOCKED      1000
#define UNLOCKED    2000
#define DEV_UP      1
#define DEV_DOWN    0

#define MY_MAJOR		42
#define MY_MINOR		0
#define NUM_MINORS		1
#define MODULE_NAME		"mahsan_cdev"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif

struct mah_device_data {
    struct cdev cdev;
    char buffer[BUFSIZ];
    ssize_t size;
    atomic_t is_locked;
    atomic_t is_up;
};

struct mah_device_data devs[NUM_MINORS];

static int mah_cdev_open(struct inode *inode, struct file *file) {
	struct mah_device_data *data;
    int flag;
    pr_info("MAHSAN: Trying to open device mahsan_cdev.\n");

    data = container_of(inode->i_cdev, struct mah_device_data, cdev);
	file->private_data = data;

    flag = atomic_cmpxchg(&data->is_locked, UNLOCKED, LOCKED);
    if(flag == LOCKED){
        pr_info("MAHSAN: Device is open in another process.\n");
        return -EBUSY;
    }
    
    pr_info("MAHSAN: Device is open successfully. Waiting a little bit...\n");
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);

	return 0;
}

static int mah_cdev_release(struct inode *inode, struct file *file) {
	struct mah_device_data *data =
		(struct mah_device_data *) file->private_data;

    atomic_set(&data->is_locked, UNLOCKED);
    pr_info("MAHSAN: Successfully closed the device.\n");
	return 0;
}

static ssize_t mah_cdev_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset) {
	struct mah_device_data *data =
		(struct mah_device_data *) file->private_data;
	ssize_t to_read;
    int flag;

    flag = atomic_read(&data->is_up);
    if(flag==DEV_DOWN){
        pr_info("MAHSAN: Device is down. Cannot read anything.\n");
        return -ENODEV;
    }

    if(*offset >= data->size){
        return 0;
    }
    to_read = min((ssize_t)size, (ssize_t)(data->size - *offset));
    copy_to_user(user_buffer, data->buffer + *offset, to_read);
    *offset += to_read;

    pr_info("MAHSAN: Reading %ld bytes from char device.\n", to_read);
	return to_read;
}

static ssize_t mah_cdev_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset) {
	struct mah_device_data *data =
		(struct mah_device_data *) file->private_data;
    ssize_t to_write;
    int flag;
    
    flag = atomic_read(&data->is_up);
    if(flag==DEV_DOWN){
        pr_info("MAHSAN: Device is down. Cannot write anything.\n");
        return -ENODEV;
    }

    if(*offset >= size)
        return 0;

    to_write = min((ssize_t)size, (ssize_t)BUFSIZ);
    copy_from_user(data->buffer, user_buffer, to_write);
    data->buffer[to_write-1] = 0;
    data->size = to_write;
    *offset += to_write;
   
    pr_info("MAHSAN: Writing %ld bytes to char device.\n", to_write);
	return to_write;
}

static long mah_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	struct mah_device_data *data =
		(struct mah_device_data *) file->private_data;
	int ret = 0;

	switch (cmd) {
        case MY_IOCTL_PRINT:
            pr_info("MAHSAN: IOCTL print is called. Here is the buffer: %s\n", data->buffer);
            break;
        case MY_IOCTL_UP:
            pr_info("MAHSAN: IOCTL UP is called. Bringing the device up and running.\n");
            atomic_set(&data->is_up, DEV_UP);
            break;
        case MY_IOCTL_DOWN:
            pr_info("MAHSAN: IOCTL DOWN is called. Shutting down the device.\n");
            atomic_set(&data->is_up, DEV_DOWN);
            break;
	default:
		ret = -EINVAL;
	}

    return ret;
}

static const struct file_operations mah_fops = {
	.owner = THIS_MODULE,
    .open = mah_cdev_open,
    .read = mah_cdev_read,
    .write = mah_cdev_write,
    .release = mah_cdev_release,
    .unlocked_ioctl = mah_cdev_ioctl
};

static int mah_cdev_init(void) {
	int err;
	int i;

    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), NUM_MINORS, MODULE_NAME);
    if(err){
        pr_err("MAHSAN: Error creating char device. code: %d\n", err);
        return err;
    }

	for (i = 0; i < NUM_MINORS; i++) {
        devs[i].buffer[0] = 0;
        devs[i].size = 0;
        atomic_set(&devs[i].is_locked, UNLOCKED);
        atomic_set(&devs[i].is_up, DEV_UP);

        cdev_init(&devs[i].cdev, &mah_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
	}
    
    pr_info("MAHSAN: Successfully initialized char device.\n");
	return 0;
}

static void mah_cdev_exit(void) {
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
        cdev_del(&devs[i].cdev);
	}

    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), NUM_MINORS);
    pr_info("MAHSAN: Successfully closed char device.\n");
}

module_init(mah_cdev_init);
module_exit(mah_cdev_exit);


