#include <linux/kprobes.h>
#include <asm/unistd.h>
#include <linux/mutex.h>

#include "syscall_wrapper.h"
#include "utils.h"

// TODO -- is 1 << 22 Too big???
// Maybe we should just save pointer to heap in kfifo?
#define SYSCALL_FIFO_ORDER (22)
#define SYSCALL_FIFO_SIZE (1 << SYSCALL_FIFO_ORDER)

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


// TODO -- what if multiple processes start syscall?
struct syscall_record *current_syscall_record;

DEFINE_MUTEX(recorded_syscalls_mutex);
struct kfifo recorded_syscalls;
DECLARE_WAIT_QUEUE_HEAD(recorded_syscalls_wait);

struct kretprobe syscall_kretprobe = {
	.kp.symbol_name	= "do_syscall_64",
	.entry_handler 	= pre_syscall,
	.handler		= post_syscall,
	// TODO: Understand why after execve check still need maxactive...
	.maxactive		= 1000
};

int pre_syscall(struct kretprobe_instance * probe, struct pt_regs *regs) {

    struct pt_regs * userspace_regs_ptr = (struct pt_regs *) regs->si;

    // We don't want to hook the return of execve \ exit because they never return :)
    IF_TRUE_CLEANUP(__NR_execve == regs->di || __NR_exit == regs->di);

	// TODO: RECORD ONLY SPECIFIC PROCESSES
	IF_TRUE_CLEANUP(0 != strcmp(current->comm, "python"));

    IF_TRUE_CLEANUP(NULL != current_syscall_record, "Multiple syscall recording the same time not supported yet");

    current_syscall_record = kmalloc(sizeof(struct syscall_record), GFP_KERNEL);
    IF_TRUE_CLEANUP(NULL == current_syscall_record, "Failed to alloc syscall record!");

    INIT_LIST_HEAD(&(current_syscall_record->copies_to_user));
    memcpy(&(current_syscall_record->userspace_regs), userspace_regs_ptr, sizeof(struct pt_regs));
    current_syscall_record->userspace_regs_ptr = userspace_regs_ptr;

	return 0;

cleanup:
	return 1;
}

int post_syscall(struct kretprobe_instance *probe, struct pt_regs *regs) {

    IF_TRUE_CLEANUP(NULL == current_syscall_record, "Current syscall not defined!");

    current_syscall_record->ret = current_syscall_record->userspace_regs_ptr->ax;

    mutex_lock(&recorded_syscalls_mutex);
    IF_TRUE_GOTO(0 == kfifo_in(&recorded_syscalls, &current_syscall_record,
                        sizeof(void *)),
                        cleanup_mutex,
                        "Failed to insert syscall to fifo!");

cleanup_mutex:
    mutex_unlock(&recorded_syscalls_mutex);

cleanup:
    kfree(current_syscall_record);
    current_syscall_record = NULL;

	return 0;
}

int init_syscall_hook(void) {

	IF_TRUE_CLEANUP(0 > register_kretprobe(&syscall_kretprobe), "Failed to init syscall kprobe!");

    IF_TRUE_GOTO(kfifo_alloc(&recorded_syscalls, SYSCALL_FIFO_SIZE, GFP_KERNEL),
                    cleanup_unregister_kprobe, "Failed to alloc kfifo!");
    return 0;

cleanup_unregister_kprobe:
    unregister_kretprobe(&syscall_kretprobe);

cleanup:
    return -1;
}

void remove_syscall_hook(void) {
    unregister_kretprobe(&syscall_kretprobe);
}