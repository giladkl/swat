#include <linux/kprobes.h>

#include "utils.h"
#include "syscall_wrapper.h"

int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs) {
    
    void * ptr;
    
    IF_TRUE_CLEANUP(strcmp(current->comm, "python"));
    IF_TRUE_CLEANUP(!is_in_syscall);


    // Simulate a memcpy to save data...
    ptr = kmalloc(regs->dx, GFP_KERNEL);
    memcpy(ptr, (void *) regs->si, regs->dx);
    kfree(ptr);

	return 0;

cleanup:
    return 1;
}

int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs) {
    // TODO Actually copy data after we see copy succeeded...
	return 0;
}
