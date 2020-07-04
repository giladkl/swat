#include <linux/kprobes.h>
#include <asm/unistd.h>

#include "utils.h"

int is_in_syscall = 0;

int pre_syscall(struct kretprobe_instance * probe, struct pt_regs *regs) {

    // We don't want to hook the return of execve \ exit because they never return :)
    IF_TRUE_CLEANUP(__NR_execve == regs->di || __NR_exit == regs->di);

	// TODO: RECORD ONLY SPECIFIC PROCESSES
	IF_TRUE_CLEANUP(strcmp(current->comm, "python"));

	is_in_syscall = 1;
	return 0;

cleanup:
	return 1;
}

int post_syscall(struct kretprobe_instance *probe, struct pt_regs *regs) {
	
    is_in_syscall = 0;
	return 0;
}
