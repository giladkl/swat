#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

#include "utils.h"
#include "syscall_wrapper.h"
#include "copy_to_user_wrapper.h"


MODULE_LICENSE("GPL");

struct kretprobe syscall_kretprobe = {
	.kp.symbol_name	= "do_syscall_64",
	.entry_handler 	= pre_syscall,
	.handler		= post_syscall,
	// TODO: Understand why after execve check still need maxactive...
	.maxactive		= 1000
};

struct kretprobe copy_kretprobe = {
	.kp.symbol_name	= "_copy_to_user",
	.entry_handler 	= pre_copy,
	.handler		= post_copy,
	.maxactive		= 1000
};

int __init swat_init(void) {
	int ret;

	ret = register_kretprobe(&syscall_kretprobe);
	IF_TRUE_CLEANUP(ret < 0, "Failed to init syscall kprobe! ret: %d\n", ret);
	
	ret = register_kretprobe(&copy_kretprobe);
	IF_TRUE_CLEANUP(ret < 0, "Failed to init copy kprobe! ret: %d\n", ret);

cleanup:
	return ret;
}

void __exit swat_exit(void) {
	unregister_kretprobe(&syscall_kretprobe);
	unregister_kretprobe(&copy_kretprobe);
	return;
}

module_init(swat_init);
module_exit(swat_exit);