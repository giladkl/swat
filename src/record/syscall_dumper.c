#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#include "utils.h"
#include "syscall_wrapper.h"

#define PROC_NAME "syscall_dumper"

struct proc_dir_entry * proc_ent = NULL;

ssize_t read(struct file *file, char __user *buf, size_t size, loff_t *ppos);

struct file_operations proc_ops = 
{
	.read = read,
};

ssize_t read(struct file *file, char __user *buf, size_t size, loff_t *ppos) {
    DEFINE_WAIT(wait);
    
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