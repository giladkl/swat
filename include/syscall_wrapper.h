#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

/*
 * @purpose:    Record all system calls recorded processes do.
 * 
 * @notes:
 *      - Runs at the beggining of every syscall called on system.
 *      - The pre_syscall function should just record syscall params,
 *          only on post_syscall we record entire syscall (only then we
 *          have the return value of syscall)
 * 
 *  * @ret: 
 *      1 == run post_syscall after syscall is done
 *      0 == don't run post_syscall after syscall
 */
int pre_syscall(struct kretprobe_instance * probe, struct pt_regs *regs);

/*
 * @purpose: Runs AFTER selected syscalls where run - and records entire syscall
 *              (with return values)
 */
int post_syscall(struct kretprobe_instance *probe, struct pt_regs *regs);

/* A var to indicate if *python* code is currently in syscall */
extern int is_in_syscall;

#endif