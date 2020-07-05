#ifndef SYSCALL_DUMPER_H
#define SYSCALL_DUMPER_H

__packed struct syscall_dump {
    struct pt_regs userspace_regs;
    unsigned long ret;
    unsigned long copies_amount;
    struct copy_record copies[];
};

int init_syscall_dumper(void);
void remove_syscall_dumper(void);

#endif