/* See LICENSE file for license and copyright information */

#include <asm/tlbflush.h>
#include <drm/drm_cache.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/memory.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#include "kernel_write.h"

MODULE_AUTHOR("Daniel Moghimi");
MODULE_DESCRIPTION("Device to let kernel write to an address");
MODULE_LICENSE("GPL");


unsigned char __attribute__((aligned(PAGE_SIZE))) buffer[PAGE_SIZE];

static int device_open(struct inode *inode, struct file *file) {
  return 0;
}

static int device_release(struct inode *inode, struct file *file) {
  return 0;
}


static long write_offset_once(unsigned long offset) {
  uint8_t * ptr = buffer + offset;
  asm volatile("incl 0(%0)\n" : : "c"(ptr) : "rax"); 
  *ptr = 'K';
  return 0;
}
static long device_ioctl(struct file *file, unsigned int ioctl_num,
                         unsigned long ioctl_param) {
  switch (ioctl_num) {
    case KERNEL_WRITE_IOCTL_CMD_WRITE_OFFSET_ONCE:
      return write_offset_once(ioctl_param);
    default:
      return -1;
  }

  return 0;
}

static struct file_operations f_ops = {
  .unlocked_ioctl = device_ioctl,
  .open = device_open,
  .release = device_release
};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = KERNEL_WRITE_DEVICE_NAME,
    .fops = &f_ops,
    .mode = S_IRWXUGO,
};

int init_module(void) {
  int r;
  memset(buffer, 'B', PAGE_SIZE);
  printk(KERN_ALERT "[kernel-write-module] Base address at %px\n", buffer);

  /* Register device */
  r = misc_register(&misc_dev);
  if (r != 0) {
    printk(KERN_ALERT "[kernel-write-module] Failed registering device with %d\n",
           r);
    return 1;
  }

  printk(KERN_INFO "[kernel-write-module] Loaded.\n");

  return 0;
}

void cleanup_module(void) {
  misc_deregister(&misc_dev);
  printk(KERN_INFO "[kernel-write-module] Removed.\n");
}
