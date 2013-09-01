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
//#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "cdata_ioctl.h"
#include "linux/slab.h"
#include "linux/wait.h"

#include "linux/workqueue.h"

//#define IO_MEM 0xe0000000
//#define VGA_MODE_WIDTH 640
//#define VGA_MODE_HEIGHT 480
//#define VGA_MODE_BPP 8

#define IO_MEM 0x33f00000
#define VGA_MODE_WIDTH 240
#define VGA_MODE_HEIGHT 320
#define VGA_MODE_BPP 32
#define BUF_SIZE 1024
#define RESERVED_ADR 0x33f00000

extern int schedule_work_on(int cpu, struct work_struct *work);


//*imortant !!!!
struct cdata_t {
	char *buf;
	unsigned int index;
	unsigned char *fbmem;
	unsigned char *fbmem_start,*fbmem_end;
	//unsigned int fbmem_index;
	wait_queue_head_t wq;
	work_struct ws;
	struct timer_list timer;
};

static int cdata_open(struct inode *inode, struct file *filp)
{
	//	while(1)	
	//	    schedule();
	//need call schedule meachine to prevent this problem.
	struct cdata_t *cdata;

	//unsigned int *fbmem;
	//int i;

	cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t),GFP_KERNEL);//GFP_KERNEL has included "schedule()" and it's support reentrant!	
	cdata->buf=(char *)kmalloc(BUF_SIZE,GFP_KERNEL); //kmalloc can provide a private space for differnet reentrant progarm. 
	cdata->index=0;
	init_waitqueue_head(&cdata->wq); //refrence OS ch4 representation of process schedule. add program to wait queue
	INIT_WORK(&cdata->ws,flush_buffer);
	//static inline void __iomem *ioremap(phys_addr_t offset, unsigned long size)
	//init_timer(&cdata->timer);
	cdata->fbmem_start=(unsigned int *)ioremap(IO_MEM,VGA_MODE_WIDTH
		* VGA_MODE_HEIGHT
		* VGA_MODE_BPP
		/ 8);
	//cdata->fbmem_start=cdata->fbmem;
	cdata->fbmem_end=cdata->fbmem_start+VGA_MODE_WIDTH*VGA_MODE_HEIGHT*VGA_MODE_BPP/8;	

	cdata->fbmem=cdata->fbmem_start;
	//cdata->fbmem_end=cdata->fbmem_start+(VGA_MODE_WIDTH*VGA_MODE_HEIGHT/8);	
	//cdata->fbmem_index=cdata->fbmem_start;
	printk(KERN_INFO "cdata_start=%d",(unsigned int)cdata->fbmem_start);
	printk(KERN_INFO "cdata_end=%d",(unsigned int)cdata->fbmem_end);

	//for (i=0;i<5000;i++) {
	//printk(KERN_INFO "test");
	//static inline void __raw_writeb(u8 b, volatile void __iomem *addr)
	//writel(0x00000033,fbmem++); //all of driver just can write & read as virtual address , in this case , we define the fbeme as a point, so if we wanna use writeb 					(write bytes) we need give the address to the writeb second parameter!!.
	//}
	
	init_timer(&cdata->timer);
	filp->private_data=(void *)cdata;
	//printk(KERN_INFO "cdata_open filp_addr= %08x\n", filp);
	return 0;
}

static ssize_t cdata_read(struct file *filp, char *buf, size_t size, loff_t *off)
//static size_t cdata_read(struct file *filp, char __user *buf,size_t size,loff_t *off)
{
	return 0;	
}
void flush_buffer(unsigned long priv)
{
	//container_of();
	struct cdata_t *cdata =(struct cdata_t *)priv;
	//unsigned char *ioaddr;
	unsigned char *ioaddr;
	int i;
	ioaddr=(unsigned char *)cdata->fbmem;
	//if ( cdata->fbmem < cdata->fbmem_end)
	//{
    for (i = 0; i < BUF_SIZE; i++) {
        if (ioaddr >= cdata->fbmem_end)
            ioaddr = cdata->fbmem_start;

        writeb(cdata->buf[i], ioaddr++);
    }
	//for(i=0;i<BUF_SIZE;i++) {
	//	if ( ioaddr < cdata->fbmem_end )
	//	{	
	//	writeb(cdata->buf[i],ioaddr++);
	//	//writel(0x00000033,ioaddr++);
	//	//ioaddr++;
	//	//cdata->fbmem++;
	//	}
	//	else
	//	{
	//	//cdata->fbmem=cdata->fbmem_start;
	//	ioaddr=cdata->fbmem_start;
	//	break;
	//	}
		
	//}
	
	//cdata->index=0;		
       	//wake_up(&cdata->wq);	
	//cdata->fbmem_index+=BUF_SIZE;
	cdata->fbmem=ioaddr;
	cdata->index=0;		
       	wake_up(&cdata->wq);	
	//}
}

static ssize_t cdata_write(struct file *filp, const char *buf, size_t size,
loff_t *off)
//static ssize_t cdata_write(struct file *filp,const char __user *buf,size_t size,loff_t *off)
{
	// process context = cause the user space program call then running kernel code we call that are process context.
	struct cdata_t *cdata =(struct cdata_t *)filp->private_data;
	struct timer_list *timer=&cdata->timer;
	unsigned int index=cdata->index;
	int i;
	for (i=0;i<size;i++) {
		if (index >= BUF_SIZE) { //reschedule !! system os have running and waiting.
			
			printk(KERN_INFO "cdata over 1024i\n");
			//interruptible_sleep_on_timeout(&cdata->wq,1*HZ); //rescheduling
			//index=0;
			//timer->function=flush_buffer;
			//timer->expires = jiffies + 1;
			//timer->function = flush_lcd;
			//timer->data = (unsigned long)cdata;
			//add_timer(timer);
			//interruptible_sleep_on(&cdata->wq);	
			schedule_work_on(1,&cdata->wt);
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
	del_timer(&cdata->timer);
	return 0;
}
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
static int cdata_ioctl(struct inode *inode, struct file *filp,
unsigned int cmd, unsigned long arg)
//static long cdata_ioctl(struct file *filp,
//                unsigned int cmd, unsigned long arg)
{
	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	unsigned int index=cdata->index;
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


static int cdata_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long start = vma->vm_start;
    unsigned long end = vma->vm_end;
    unsigned long total_size,page_size;
    unsigned long page;
    unsigned long index=0;
    page=RESERVED_ADR;
    page_size=PAGE_SIZE;
    total_size = end - start;
     
    //remap_page_range(start, 0x33f00000, size, PAGE_SHARED);
    while (index <= total_size)
	{
    	//remap_page_range(start, 0x33f00000, PAGE_SIZE, PAGE_SHARED);
    	remap_page_range(start, page, PAGE_SIZE, PAGE_SHARED);
	index+=PAGE_SIZE; 
	start+=PAGE_SIZE;
	page+=PAGE_SIZE;
	}
}



	//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *); operations 
static struct file_operations __cdata_fops = {
	//kernel 2.4 before version,no owner to conunt module used conunt , 2.6 > will support it!
	owner:		THIS_MODULE, 
	open:		cdata_open,
	read:		cdata_read,
	write:		cdata_write,
	ioctl:          cdata_ioctl,
	//unlocked_ioctl:          cdata_ioctl,
	release:	cdata_close,
	mmap:		cdata_mmap,
};

static struct miscdevice cdata_fops={
	minor: 	12,
	name:	"cdata",
	fops:	&__cdata_fops,

};
static int cdata_init_module(void)
{
//	register_chrdev(121,"cdata",&cdata_fops); //register device API , it will register to /dev/ major numer  (120 ~ 127 is reserved for development)
//	return 0;
	if (misc_register(&cdata_fops)) {
		printk(KERN_ALERT "cdata:register failed\n");
		return -1;
	}
	printk(KERN_ALERT "cdata driver: registering ...\n");
	return 0;
}

static void cdata_cleanup_module(void)
{
	//unregister_chrdev(121,"cdata");
	misc_deregister(&cdata_fops);

}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
