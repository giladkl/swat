#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

#include "utils.h"
#include "syscall_wrapper.h"
#include "copy_to_user_wrapper.h"
#include "syscall_dumper.h"

MODULE_LICENSE("GPL");

static int __init recorder_init(void) {

	IF_TRUE_CLEANUP(init_syscall_hook(), "Failed to init syscall hook");
	IF_TRUE_GOTO(init_copy_hook(), cleanup_syscall_hook, "Failed to init syscall hook");
	IF_TRUE_GOTO(init_syscall_dumper(), cleanup_copy_hook, "Failed to init syscall dumper!");

	return 0;

cleanup_copy_hook:
	remove_copy_hook();
cleanup_syscall_hook:
	remove_syscall_hook();
cleanup:
	return -1;
}

static void __exit recorder_exit(void) {
	// TODO WHAT HAPPENS IF WE REMOVE WHILE IN HOOK?
	remove_syscall_dumper();
	remove_copy_hook();
	remove_syscall_hook();
	return;
}

module_init(recorder_init);
module_exit(recorder_exit);
