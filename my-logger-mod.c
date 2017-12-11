#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hiziri")
MODULE_DESCRIPTION("This Module is Test Code.");
MODULE_VERSION("0.1");

static char *module_name = "My-Key-Logger";
module_param(module_name, charp, S_IRUGO);

static int __init my_key_logger_init(void){
  printk(KERN_INFO "MKL: %s is Loaded!", module_name);
  return 0;
}

static void __exit my_key_logger_init(void){
  printk(KERN_INFO "MKL: %s is Unloaded!", module_name);
}

module_init(my_key_logger_init);
module_exit(my_key_logger_exit);
