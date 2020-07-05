#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#include "utils.h"
#include "syscall_wrapper.h"
#include "copy_to_user_wrapper.h"
#include "syscall_dumper.h"

#define PROC_NAME "syscall_dumper"

struct proc_dir_entry * proc_ent = NULL;

ssize_t read(struct file *file, char __user *buf, size_t size, loff_t *ppos);

struct file_operations proc_ops = {
	.read = read,
};

unsigned long get_record_len(struct syscall_record *syscall_record) {
    
    unsigned long size = sizeof(struct syscall_dump);
    struct copy_record_element *copy_record_element;

    list_for_each_entry(copy_record_element, &(syscall_record->copies_to_user), list) {
        size += sizeof(struct copy_record) + copy_record_element->record.len;
    }
    
    return size;
}

ssize_t dump_record_to_user(char __user *buf, size_t size) {
    struct syscall_record *syscall_record;
    struct syscall_dump syscall_dump;
    unsigned long len;
    unsigned long cur_write = 0;
    struct copy_record_element *copy_record_element;

    IF_TRUE_CLEANUP(
        sizeof(void *) !=
        kfifo_out(&recorded_syscalls, &(syscall_record), sizeof(void *)),
        "Failed to read from fifo enough elements :(");

    len = get_record_len(syscall_record);
    IF_TRUE_CLEANUP(size < len, "Size for copy too small");
    
    syscall_dump.copies_amount = 0;
    cur_write += sizeof(syscall_dump);

    list_for_each_entry(copy_record_element, &(syscall_record->copies_to_user), list) {
        syscall_dump.copies_amount++;
        copy_to_user(buf + cur_write, &(copy_record_element->record), sizeof(struct copy_record) + copy_record_element->record.len);
    }

    memcpy(&syscall_dump.userspace_regs, &syscall_record->userspace_regs, sizeof(struct pt_regs));
    syscall_dump.ret = syscall_record->ret;

    // TODO FREEEEEEEEEE IT ALLLLLLLLLL
    IF_TRUE_CLEANUP(0 != copy_to_user(buf, &syscall_dump, sizeof(syscall_dump)), "Failed to copy to user syscall");

    return len;

cleanup:
    return -EINVAL;
}

ssize_t read(struct file *file, char __user *buf, size_t size, loff_t *ppos) {
    DEFINE_WAIT(wait);
    ssize_t ret;
    
    add_wait_queue(&recorded_syscalls_wait, &wait);
    while (1)
    {
        mutex_lock(&recorded_syscalls_mutex);

        if (kfifo_is_empty(&recorded_syscalls))
        {
            mutex_unlock(&recorded_syscalls_mutex);
            prepare_to_wait(&recorded_syscalls_wait, &wait, TASK_INTERRUPTIBLE);

            schedule();
            
            // If woken up from signal
            IF_TRUE_GOTO(signal_pending(current), cleanup_signal);

            continue;
        }

        break;
        // Wait for syscalls...
    }
    finish_wait(&recorded_syscalls_wait, &wait);

    ret = dump_record_to_user(buf, size);
    mutex_unlock(&recorded_syscalls_mutex);

    return ret;

cleanup_signal:
    finish_wait(&recorded_syscalls_wait, &wait);
    return -ERESTARTSYS;
}

int init_syscall_dumper(void) {
    // TODO WTF IS THE RIGHT PERMISSIONS?
    proc_ent = proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    IF_TRUE_CLEANUP(NULL == proc_ent, "Failed to create proc entry!");

    return 0;
cleanup:
    return -1;
}

void remove_syscall_dumper(void) {

}