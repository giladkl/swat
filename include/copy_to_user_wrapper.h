#ifndef COPY_TO_USER_WRAPPER_H
#define COPY_TO_USER_WRAPPER_H

/*
 * @purpose: Record all data kernel copies to userspace processes we record
 * 
 * @notes: 
 *      - Runs at the beggining of every copy_to_user called on system.
 *      - The pre_copy function should just record copy params and not save
 *          them yet, because only in post_copy we know if copy suceeded.
 * 
 * @ret: 
 *      1 == run post_syscall after syscall is done
 *      0 == don't run post_syscall after syscall
 */
int pre_copy(struct kretprobe_instance * probe, struct pt_regs *regs);

/*
 * @purpose: Runs AFTER copy_to_user, and if it was successfull,
 *              record copied data for recording..
 */
int post_copy(struct kretprobe_instance *probe, struct pt_regs *regs);

#endif
