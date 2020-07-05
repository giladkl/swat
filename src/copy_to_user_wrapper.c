#include <linux/kprobes.h>
#include <linux/list.h>

#include "utils.h"
#include "copy_to_user_wrapper.h"
#include "syscall_wrapper.h"

/*
 * @purpose: Record all data kernel copies to userspace processes we record
 * 
 * @notes: 
 *      - Runs at the beggining of every copy_to_user called on system.
 *      - The pre_copy function should just record copy params and not save
 *          them yet, because only in post_copy we know if copy suceeded.
 * 
 * @ret: 
 *      1 == run post_copy after syscall is done
 *      0 == don't run post_copy after syscall
 */
int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs);

/*
 * @purpose: Runs AFTER copy_to_user, and if it was successfull,
 *              record copied data for recording..
 */
int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs);


struct kretprobe copy_kretprobe = {
	.kp.symbol_name	= "_copy_to_user",
	.entry_handler 	= pre_copy,
	.handler		= post_copy,
	.maxactive		= 1000
};

struct copy_record * current_copy = NULL;

int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs) {
        
    IF_TRUE_CLEANUP(strcmp(current->comm, "python"));
    IF_TRUE_CLEANUP(!is_in_syscall);
    IF_TRUE_CLEANUP(NULL != current_copy, "ERROR! Can only record one copy at the same time!");
    
    current_copy = kmalloc(sizeof(struct copy_record), GFP_KERNEL);
    IF_TRUE_CLEANUP(NULL == current_copy, "Failed to alloc current copy!");

    current_copy->from = (void *) regs->si;
    current_copy->to = (void *) regs->di;
    current_copy->len = (unsigned long) regs->dx;

	return 0;

cleanup:
    return 1;
}

int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs) {

    IF_TRUE_CLEANUP(regs_return_value(regs), "copy_to_user failed! not saving.");
    IF_TRUE_CLEANUP(0 == current_copy->len, "Trying to make copy of 0?");

    current_copy->bytes = kmalloc(current_copy->len, GFP_KERNEL);
    IF_TRUE_CLEANUP(NULL == current_copy->bytes, "Failed to alloc bytes copy!");

    memcpy(current_copy->bytes, current_copy->from, current_copy->len);

    list_add_tail(&current_copy->list, &current_syscall_context.recorded_syscall.copies_to_user);

cleanup:
    kfree(current_copy);
    current_copy = NULL;

	return 0;
}

int init_copy_hook(void) {
	IF_TRUE_CLEANUP(0 > register_kretprobe(&copy_kretprobe), "Failed to init syscall kprobe!");

    return 0;
cleanup:
    return -1;
}

void remove_copy_hook(void)
{
   	unregister_kretprobe(&copy_kretprobe);
}