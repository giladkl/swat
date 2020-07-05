#include <linux/kernel.h>

inline void log_if_not_null(char *fmt, ...) {
   	va_list args;

    if (fmt)
    {
        va_start(args, fmt);
        vprintk(fmt, args);
        va_end(args);

        printk("\n");
    }
}
