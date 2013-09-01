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
#include <asm/io.h>
#include <asm/uaccess.h>

#include "cdata_ioctl.h"

#define IO_MEM              0x33f00000
#define VGA_MODE_WIDTH      240
#define VGA_MODE_HEIGHT     320
#define VGA_MODE_BPP        32
#define BUF_SIZE            1024

struct cdata_t {
    char *buf;
    unsigned int        index;
    wait_queue_head_t   wq;
    unsigned char        *fbmem;
    unsigned char        *fbmem_start, *fbmem_end;
    //struct semaphore sem;
    struct mutex *lock;
    struct work_struct work;
    struct spinlock_t *fastlock;
 
};
// Global lock 
DEFINE_MUTEX(cdata_sem);


void flush_buffer((struct work_struct *work)
{
    struct cdata_t *cdata = container_of(work, struct cdata_t, work);
    unsigned char *ioaddr;
    int i;
    char d;

    ioaddr = (unsigned char *)cdata->fbmem;
    // could call scheduler to reschedule.

   // spin_lock_irq(&cdata->fastlock);
    for (i = 0; i < BUF_SIZE; i++) {
        if (ioaddr >= cdata->fbmem_end)
            ioaddr = cdata->fbmem_start;
	d=atomic_read(d,ioaddr++);
        writeb(d,ioaddr++);
       // writeb(cdata->buf[i], ioaddr++); // just care any interrupt active, so just add spin lock irq.
    }


    //spin_unlock_irq(&cdata->faselock);

    cdata->fbmem = ioaddr;

    //mutex_lock(&cdata->lock); mutex time is bigger than to long
    //spin_lock(); spin lock couldn't protect a stat that interrupt action when cdata->index operation.
    spin_lock_irq(&cdata->fastlock);
    cdata->index = 0; // if use atomic operation to make cdata->index = 0 , the sping_lock is not necessary to have .
    spin_unlock_irq(&cdata->faselock);
    //spin_unlock();
    //mutex_unlock(&cdata->lock);

    wake_up(&cdata->wq);
}

static int cdata_open(struct inode *inode, struct file *filp)
{
    struct cdata_t *cdata;
    int i;
    //mutex_lock(&cdata_sem);
    cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
    cdata->buf = (char *)kmalloc(BUF_SIZE, GFP_KERNEL);

    cdata->index = 0;
    init_waitqueue_head(&cdata->wq);
	
    //sema_init(&cdata->sem, 0);
    mutex_init(&cdata->lock);
    INIT_WORK(&cdata->work, flush_buffer);

    cdata->fbmem_start = (unsigned int *) 
            ioremap(IO_MEM, VGA_MODE_WIDTH
                    * VGA_MODE_HEIGHT
                    * VGA_MODE_BPP
                    / 8);

    cdata->fbmem_end = cdata->fbmem_start + VGA_MODE_WIDTH 
                                      * VGA_MODE_HEIGHT
                                      * VGA_MODE_BPP
                                      / 8;

    cdata->fbmem = cdata->fbmem_start;

    filp->private_data = (void *)cdata;

    printk(KERN_INFO "in cdata_open: filp = %08x\n", filp);

   // mutex_unlock(&cdata_sem); it's non necessary .. cause open will keep own data.
    return 0;
}

static ssize_t cdata_read(struct file *filp, char *buf, size_t size, loff_t *off)
{
    return 0;
}
//
/*
Sempahore must care below thing .
1.local variable
2.atomic operation
3.SMP support (avoid IO reordering ,use memory barrier)
4.Code review.

*/



static ssize_t cdata_write(struct file *filp, const char *buf, size_t size, loff_t *off)
{
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    unsigned int index;
    wait_queue_t wait;
    int i;

    // NOTE: put shared data into local variables
    //down_interruptible(&cdata->sem);
	
    mutex_lock(&cdata->lock);
    index = cdata->index;

    // NOTE: share the same memory space
    //wq = &cdata->wq;
    mutex_unlock(&cdata->lock);
    //up(&cdata->sem);

    for (i = 0; i < size; i++) {
        if (index >= BUF_SIZE) {
            printk(KERN_INFO "cdata: buffer full\n");

            schedule_work_on(1, &cdata->work);

            wait.flags = 0;
            wait.task = current;

            // NOTE: must be atomic operation
            add_wait_queue(&cdata->wq, &wait);
repeat:
            //current->state = TASK_INTERRUPTIBLE;
            set_current_state(TASK_INTERRUPTIBLE);
            schedule()

    	    mutex_lock(&cdata->lock);
	    //down_interruptible(&cdata->sem);
            index = cdata->index;
	   // up(&cdata->sem);

    	    mutex_unlock(&cdata->lock);

            if (index != 0)
                goto repeat;

            remove_wait_queue(wq, &wait);
        }
        copy_from_user(&cdata->buf[index], &buf[i], 1); // almost kernel api are atomic program
        index++;
    }

    //down_interruptible(&cdata->sem);
    mutex_lock(&cdata->lock);
    cdata->index = index;
    //up(&cdata->sem);
    mutex_unlock(&cdata->lock);

    return 0;
}

static int cdata_close(struct inode *inode, struct file *filp)
{
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    unsigned int index = cdata->index;

    cdata->buf[index] = '\0';

    printk(KERN_INFO "in cdata_close: %s\n", cdata->buf);

    return 0;
}

int cdata_ioctl(struct inode *inode, struct file *filp, 
                    unsigned int cmd, unsigned long arg)
{
    struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
    unsigned int index = cdata->index;

    switch (cmd) {
        case CDATA_EMPTY:
            index = 0;
            break;
        case CDATA_SYNC:
            cdata->buf[index] = '\0';
            printk(KERN_INFO "str: %s\n", cdata->buf);
            break;
        case CDATA_WRITE:
            cdata->buf[index++] = *((char *)arg);
            break;
        default:
            return -1;
    }

    cdata->index = index;

    return 0;
}

static struct file_operations __cdata_fops = {    
        owner:      THIS_MODULE,
        open:       cdata_open,
        release:    cdata_close,
        read:       cdata_read,
        write:      cdata_write,
        ioctl:      cdata_ioctl,
};

static struct miscdevice cdata_fops = {
        minor:      12,
        name:       "cdata",
        fops:       &__cdata_fops,
};

int cdata_init_module(void)
{
    if (misc_register(&cdata_fops)) {
        printk(KERN_ALERT "cdata: register failed.\n");
        return -1;
    }
    printk(KERN_ALERT "cdata-fb: registered .\n");
}

void cdata_cleanup_module(void)
{
    misc_deregister(&cdata_fops);
}

module_init(cdata_init_module);
module_exit(cdata_cleanup_module);

MODULE_LICENSE("GPL");
