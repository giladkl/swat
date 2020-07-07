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

struct kretprobe copyin_kretprobe = {
	.kp.symbol_name	= "copyout",
	.entry_handler 	= pre_copy,
	.handler		= post_copy,
	.maxactive		= 1000
};

struct copy_record_element * current_copy = NULL;

int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs) {
    
    unsigned long copy_len = (unsigned long) regs->dx;

    IF_TRUE_CLEANUP(0 != strcmp(current->comm, "python"));
    IF_TRUE_CLEANUP(NULL == current_syscall_record);
    IF_TRUE_CLEANUP(NULL != current_copy, "ERROR! Can only record one copy at the same time!");
    IF_TRUE_CLEANUP(0 == copy_len, "ERROR! Trying to copy something with 0 len!");

    current_copy = kmalloc(sizeof(struct copy_record_element) + copy_len, GFP_KERNEL);
    IF_TRUE_CLEANUP(NULL == current_copy, "Failed to alloc current copy!");

    current_copy->record.from = (void *) regs->si;
    current_copy->record.to = (void *) regs->di;
    current_copy->record.len = copy_len;

	return 0;

cleanup:
    return 1;
}


int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs) {

    IF_TRUE_CLEANUP(regs_return_value(regs), "copy_to_user failed! not saving.");

    memcpy(current_copy->record.bytes, current_copy->record.from, current_copy->record.len);

    list_add_tail(&current_copy->list, &(current_syscall_record->copies_to_user));

    // If we put every thing in list like we want we don't want to free element
    goto cleanup_without_free;

cleanup:
    kfree(current_copy);

cleanup_without_free:
    current_copy = NULL;

	return 0;
}

int init_copy_hook(void) {
	IF_TRUE_CLEANUP(0 > register_kretprobe(&copy_kretprobe), "Failed to init syscall kprobe!");

    IF_TRUE_GOTO(0 > register_kretprobe(&copyin_kretprobe), cleanup_unregister, "Failed to init syscall kprobe!");

    return 0;

cleanup_unregister:
    unregister_kretprobe(&copy_kretprobe);
cleanup:
    return -1;
}

void remove_copy_hook(void)
{
   	unregister_kretprobe(&copy_kretprobe);
    unregister_kretprobe(&copyin_kretprobe);
}