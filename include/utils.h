#ifndef UTILS_H
#define UTILS_H

#define _IF_TRUE_GOTO(cond, label, ...) do {     \
        if (cond){                               \
            log_if_not_null(__VA_ARGS__);        \
            goto label;                          \
        }                                        \
    } while (0)

#define IF_TRUE_GOTO(...) _IF_TRUE_GOTO(__VA_ARGS__, 0)

#define _IF_TRUE_CLEANUP(cond, ...) do {        \
        if (cond){                              \
            log_if_not_null(__VA_ARGS__);       \
            goto cleanup;                       \
        }                                       \
    } while (0)

#define IF_TRUE_CLEANUP(...) _IF_TRUE_CLEANUP(__VA_ARGS__, 0)

/*
 * @purpose: log received format if format is not NULL...
 * 
 * @notes:
 *          - Since I hate adding \n to my str,
 *               it always appends \n to your fmt
 */
inline void log_if_not_null(char * fmt, ...);

#endif