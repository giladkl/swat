#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

#include <linux/kfifo.h>
#include <linux/list.h>


struct syscall_record {
    struct pt_regs userspace_regs;
    unsigned long ret;
    struct list_head copies_to_user;
};

struct syscall_hook_context {
    struct syscall_record recorded_syscall;

    /* We need to save pointer to userspace regs becuse
     * we need to read userspace eax after syscall to see
     * retcode
     */
    struct pt_regs * userspace_regs;
};

/*
 * @purpose: Hook syscalls
 */
int init_syscall_hook(void);

/*
 * @purpose: Unhool syscalls
 */
void remove_syscall_hook(void);


/* A var to indicate if *python* code is currently in syscall */
extern int is_in_syscall;
extern struct syscall_hook_context current_syscall_context;

#endif
