#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");

int pre_syscall(struct kretprobe_instance * probe, struct pt_regs *regs);
int post_syscall(struct kretprobe_instance *probe, struct pt_regs *regs);

int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs);
int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs);


struct kretprobe syscall_kretprobe = {
	.kp.symbol_name	= "do_syscall_64",
	.entry_handler 	= pre_syscall,
	.handler		= post_syscall,
	// TODO: EXECVE, EXIT HANDLE
	.maxactive		= 1000
};

struct kretprobe copy_kretprobe = {
	.kp.symbol_name	= "_copy_to_user",
	.entry_handler 	= pre_copy,
	.handler	= post_copy,
	// TODO: EXECVE, EXIT HANDLE
	.maxactive		= 1000
};

int is_in_syscall = 0;

int pre_syscall(struct kretprobe_instance * probe, struct pt_regs *regs) {

	if (0 == strcmp(current->comm, "python"))
	{
		//printk("Starting syscall number %ld", regs->di);
		is_in_syscall = 1;
	}

	return 0;
}

int post_syscall(struct kretprobe_instance *probe, struct pt_regs *regs) {
	
	if (0 == strcmp(current->comm, "python"))
	{
		is_in_syscall = 0;
	}

	return 0;
}

int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs) {

	if (0 == strcmp(current->comm, "python") && is_in_syscall)
	{
		void * from = (void *) regs->si;
		unsigned long long size = regs->dx;

		// Simulate a memcpy to save data...
		void * ptr = kmalloc(size, GFP_KERNEL);
		memcpy(ptr, from, size);
		kfree(ptr);

	}
	return 0;
}

//TODO DO WE NEED KRET PROBE HERE?
int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs) {
	return 0;
}

int __init swat_init(void) {
	int ret;

	ret = register_kretprobe(&syscall_kretprobe);
	if (ret < 0) {
		printk("Failed to init kprobe! ret: %d\n", ret);
	}

	ret = register_kretprobe(&copy_kretprobe);
	if (ret < 0) {
		printk("Failed to init kprobe! ret: %d\n", ret);
	}

	return 0;
}

void __exit swat_exit(void) {
	unregister_kretprobe(&syscall_kretprobe);
	unregister_kretprobe(&copy_kretprobe);
	return;
}

module_init(swat_init);
module_exit(swat_exit);