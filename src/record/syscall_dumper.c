#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/slab.h>

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

unsigned int dump_copy_records(struct syscall_record *syscall_record, char __user *buf) {

    struct copy_record_element *copy_record_element;
    unsigned int record_size;
    unsigned int amount_of_records = 0;

    list_for_each_entry(copy_record_element, &(syscall_record->copies_to_user), list) {
        record_size = sizeof(struct copy_record) + copy_record_element->record.len;
        IF_TRUE_CLEANUP(0 != copy_to_user(buf, &(copy_record_element->record), record_size),
                            "Failed to dump copy record!");

        buf += record_size;
        amount_of_records++;
    }

    return amount_of_records;

cleanup:
    return -1;
}

void free_syscall_record(struct syscall_record *syscall_record)
{
    struct copy_record_element *copy_record_element;
    struct copy_record_element *tmp_record_element;

    list_for_each_entry_safe(copy_record_element, tmp_record_element, &(syscall_record->copies_to_user), list) {
        kfree(copy_record_element);
    }

    kfree(syscall_record);
}

int dump_syscall_record(struct syscall_record *syscall_record, char __user *buf) {
    unsigned long amount_of_copies;
    struct syscall_dump syscall_dump;
    int ret = -1;

    // First dump copy records to know how to fill syscall_dump
    amount_of_copies = dump_copy_records(syscall_record, buf + sizeof(struct syscall_dump));
    IF_TRUE_CLEANUP(amount_of_copies < 0, "Failed to get amount of copies in syscall!");

    syscall_dump.ret = syscall_record->ret;
    syscall_dump.userspace_regs = syscall_record->userspace_regs;
    syscall_dump.copies_amount = amount_of_copies;

    IF_TRUE_CLEANUP(0 != copy_to_user(buf, &syscall_dump, sizeof(struct syscall_dump)),
                        "Failed to copy syscall record to user!");

    ret = 0;

cleanup:
    free_syscall_record(syscall_record);

    return ret;
}

#define GENERAL_ERROR 1
#define SHORT_WRITE_ERROR 2

int dump_record(char __user *buf, size_t size) {
    struct syscall_record *syscall_record;
    unsigned long len;

    IF_TRUE_CLEANUP(
        sizeof(void *) !=
        kfifo_out(&recorded_syscalls, &(syscall_record), sizeof(void *)),
        "Failed to read from fifo enough elements :(");

    len = get_record_len(syscall_record);
    IF_TRUE_GOTO(size < len, cleanup_short_write);
    
    IF_TRUE_CLEANUP(dump_syscall_record(syscall_record, buf), "Failed to dump syscall!");

    return len;
cleanup_short_write:
    return -SHORT_WRITE_ERROR;
cleanup:
    return -GENERAL_ERROR;
}

ssize_t dump_records(char __user *buf, size_t size) {
    int size_written = 0;
    int dump_ret = dump_record(buf, size);

    IF_TRUE_CLEANUP(0 > dump_ret, "Failed to dump the first record");

    buf += dump_ret;
    size -= dump_ret;
    size_written += dump_ret;

    while (1) {
        IF_TRUE_GOTO(kfifo_is_empty(&recorded_syscalls), cleanup_success);
        dump_ret = dump_record(buf, size);
        switch (dump_ret)
        {
            case -SHORT_WRITE_ERROR:
                return size_written;
                break;

            case -GENERAL_ERROR:
                goto cleanup;
                break;
            
            default:
                buf += dump_ret;
                size -= dump_ret;
                size_written += dump_ret;
        }
    }

cleanup_success:
    return size_written;
cleanup:
    return -EFAULT;
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

    ret = dump_records(buf, size);
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