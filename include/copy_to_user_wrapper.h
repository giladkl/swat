#ifndef COPY_TO_USER_WRAPPER_H
#define COPY_TO_USER_WRAPPER_H

struct copy_record
{
    void * from;
    void * to;
    unsigned long len;
    void * bytes;
    struct list_head list;
};

/*
 * @purpose: Hook copy_to_user
 */
int init_copy_hook(void);

/*
 * @purpose: Unhool copy_to_user
 */
void remove_copy_hook(void);

#endif
