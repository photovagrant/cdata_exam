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
#include "cdata_ioctl.h"
#include "linux/slab.h"
#include "linux/wait.h"
#include "linux/workqueue.h"
//*imortant !!!!
struct cdata_t {
	char *buf;
	unsigned int index;
	wait_queue_head_t wq;
	struct work_struct ws;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
	//	while(1)	
	//	    schedule();
	//need call schedule meachine to prevent this problem.
	struct cdata_t *cdata;
	cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t),GFP_KERNEL);//GFP_KERNEL has included "schedule()" and it's support reentrant!	
	cdata->buf=(char *)kmalloc(1024,GFP_KERNEL); //kmalloc can provide a private space for differnet reentrant progarm. 
	cdata->index=0;
	init_waitqueue_head(&cdata->wq); //refrence OS ch4 representation of process schedule. add program to wait queue
	//INIT_WORK(&cdata->ws,flushxxx);
	filp->private_data=(void *)cdata;
	printk(KERN_INFO "cdata_open filp_addr= %08x\n", filp);
	return 0;
}

static size_t cdata_read(struct file *filp, char __user *buf,size_t size,loff_t *off)
{
	return 0;	
}

static ssize_t cdata_write(struct file *filp,const char __user *buf,size_t size,loff_t *off)
{
	// process context = cause the user space program call then running kernel code we call that are process context.
	struct cdata_t *cdata =(struct cdata_t *)filp->private_data;
	unsigned int index=cdata->index;
	int i;
	for (i=0;i<size ;i++) {
		if (index >= 1024) { //reschedule !! system os have running and waiting.
			
			printk(KERN_INFO "cdata over 1024i\n");
			interruptible_sleep_on_timeout(&cdata->wq,1*HZ); //rescheduling
			schedule_work_on(1,&cdata->ws);
			schedule();
			index=0;
			//interruptible_sleep_on(&cdata->wq);	
			//current->state = TASK_INTERRUPTIBLE; //switch state from running to waiting
			//schedule();
			//return -1; //buffer full
		}
		copy_from_user(&cdata->buf[index],&buf[i],1);
		index++;
	}
	cdata->index=index;
	return 0;
}

static int cdata_close(struct inode *inode,struct file *filp)
{
	struct cdata_t *cdata =(struct cdata_t *)filp->private_data;
	unsigned int index=cdata->index;
	//printk(KERN_INFO "cdata_close");

	cdata->buf[index] ='\0';
	printk(KERN_INFO "cdata_closei & data= %s\n",cdata->buf);

	return 0;
}
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
static long cdata_ioctl(struct file *filp,
                unsigned int cmd, unsigned long arg)
{
	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	unsigned int index=cdata->index;

			printk(KERN_INFO "in ioctl: IOCTL\n");
	switch (cmd) {
		case IOCTL_EMPTY:
			printk(KERN_INFO "in ioctl: IOCTL_EMPTY\n");
			index=0;
			break;
		case IOCTL_WRITE:
			//buf[index++] =*((char *)arg);
			//cdata->buf[cdata->index++]=*((char *)arg); //bad method!!!
			cdata->buf[index++]=*((char *)arg); //is better that above method!!! slide #90page , dont access data from user space!
			printk(KERN_INFO "in ioctl: IOCTL_WRITE \n");
			break;
		case IOCTL_SYNC:
			cdata->buf[index] ='\0';
			printk(KERN_INFO "in ioctl: IOCTL_SYNCI %s\n",cdata->buf);
			//printk(KERN_INFO "in ioctl: IOCTL_SYNC input data= %c,%c,%c,%c\n",buf[0],buf[1],buf[2],buf[3]);
			break;
		default:
			return -1;
	}

	cdata->index=index;
	return 0;
}

	//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); operations 
static struct file_operations cdata_fops = {
	//kernel 2.4 before version,no owner to conunt module used conunt , 2.6 > will support it!
	owner:		THIS_MODULE, 
	open:		cdata_open,
	read:		cdata_read,
	write:		cdata_write,
	compat_ioctl:          cdata_ioctl,
	//unlocked_ioctl:          cdata_ioctl,
	release:	cdata_close,
};

static int cdata_init_module(void)
{
	register_chrdev(121,"cdata",&cdata_fops); //register device API , it will register to /dev/ major numer  (120 ~ 127 is reserved for development)
	return 0;
}

static void cdata_cleanup_module(void)
{
	unregister_chrdev(121,"cdata");

}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
