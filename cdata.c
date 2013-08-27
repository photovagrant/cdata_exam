#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/uaccess.h>
//#include "cdata_ioctl.h"
#include "linux/slab.h"
#include "linux/wait.h"



//static inline int register_chrdev(unsigned int major, const char *name,
//				  const struct file_operations *fops)
static int cdata_open(struct inode *inode,struct file *filp)
{
//cdata kmalloc 
printk(KERN_INFO "cdata_open");
return 0;
}


static int cdata_release(struct inode *inode, struct file *filp)
{

printk(KERN_INFO "cdata_release");
return 0;
}


static ssize_t cdata_read(struct file *filp, char __user *buf, size_t size, loff_t *loff)
{

printk(KERN_INFO "cdata_read");
return 0;
}

static ssize_t cdata_write(struct file *filp, const char __user *buf, size_t size, loff_t *loff)
{
printk(KERN_INFO "cdata_write");
return 0;
}
static long cdata_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
printk(KERN_INFO "cdata_ioctl");
return 0;
}
static struct file_operations cdata_fops ={
	owner: 	THIS_MODULE,
	open:	cdata_open,
	release:	cdata_release,
	read:	cdata_read,
	write:	cdata_write,
	compat_ioctl:	cdata_ioctl,
};


static int cdata_init_module(void)
{
	printk(KERN_INFO "cdata_init");
	register_chrdev(121,"cdata",&cdata_fops);
	return 0;
}

static void cdata_cleanup_module(void)
{

}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);
MODULE_LICENSE("GPL");
