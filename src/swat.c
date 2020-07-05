#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

#include "utils.h"
#include "syscall_wrapper.h"
#include "copy_to_user_wrapper.h"


MODULE_LICENSE("GPL");



int __init swat_init(void) {
	IF_TRUE_CLEANUP(init_syscall_hook(), "Failed to init syscall hook");
	
	IF_TRUE_CLEANUP(init_copy_hook(), "Failed to init syscall hook");

	return 0;
cleanup:
	return -1;
}

void __exit swat_exit(void) {
	remove_syscall_hook();
	remove_copy_hook();
	return;
}

module_init(swat_init);
module_exit(swat_exit);
