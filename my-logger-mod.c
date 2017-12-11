#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "mykeylogger"
#define CLASS_NAME "MKL"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hiziri");
MODULE_DESCRIPTION("This Module is Test Code.");
MODULE_VERSION("0.1");

static int majourNumber;
static char log[2048] = {0};
static int log_size;
static struct class* mkl_class = NULL;
static struct device* mkl_dev = NULL;

static char *module_name = DEVICE_NAME;
module_param(module_name, charp, S_IRUGO);

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .releas = dev_release,
};

static int __init my_key_logger_init(void){
  printk(KERN_INFO "MKL: %s is Loaded!", module_name);

  // allocate majour num
  majourNumber = register_chrdev(0, DEVICE_NAME, &fops);
  if (majourNumber < 0) {
    printk(KERN_ALERT "%s: failed to register majour num.", CLASS_NAME);
    return majourNumber;
  }
  printk(KERN_INFO "%s: registered majour num", CLASS_NAME);
  
  // register dev class
  mkl_class = register_chrdev(0, DEVICE_NAME, &fops);
  if (IS_ERR(mkl_class)) {
    unregister_chrdev(majourNumber, DEVICE_NAME);
    printk(KERN_ALERT "%s: failed to register dev class", CLASS_NAME);
    return PTR_ERR(mkl_class);
  }
  printk(KERN_INFO "%s: dev class registered", CLASS_NAME);

  // register dev driver
  mkl_dev = device_create(mkl_class, NULL, MKDEV(majourNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(mkl_dev)) {
    class_destroy(mkl_class);
    unregister_chrdev(majourNumber, DEVICE_NAME);
    printk(KERN_ALERT "%s: failed to create device", CLASS_NAME);
    return PTR_ERR(mkl_dev);
  }
  printk(KERN_INFO "%s: device class created", CLASS_NAME);
  
  return 0;
}

static void __exit my_key_logger_exit(void){
  device_destroy(mkl_class, MKDEV(majourNumber, 0));
  class_unregister(mkl_class);
  class_destroy(mkl_class);
  unregister_chrdev(majourNumber, DEVICE_NAME); 
  
  printk(KERN_INFO "MKL: %s is Unloaded!", module_name);  
}

module_init(my_key_logger_init);
module_exit(my_key_logger_exit);
