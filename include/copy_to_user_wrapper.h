#ifndef COPY_TO_USER_WRAPPER_H
#define COPY_TO_USER_WRAPPER_H

struct copy_record
{
    void * from;
    void * to;
    unsigned long len;
    unsigned char bytes[];
};

struct copy_record_element
{
    struct list_head list;
    
    // This must be last because of bytes[]
    struct copy_record record;
};

/*
 * @purpose: Hook copy_to_user
 */
int init_copy_hook(void);

/*
 * @purpose: Unhook copy_to_user
 */
void remove_copy_hook(void);

#endif
