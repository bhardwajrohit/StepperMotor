/* Replacements for C-syscalls that newlib will make.  since we don't
 * actually have a standard I/O library, these syscalls will use the
 * UARTS/USB to read and write.  It is possible here to add addional
 * code to handle multiple file descriptors, as described by the
 * newlib manual.  Here we will just follow the rule that writing to
 * any fd means we want output to the terminal.
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <reent.h>

register char *stack_ptr asm("sp");

void *_sbrk_r(struct _reent *ptr, int incr)
{
        extern char end asm("end");
        static char *heap_end;
        char *prev_heap_end;

        if (heap_end == 0)
                heap_end = &end;

        prev_heap_end = heap_end;
        if (heap_end + incr > stack_ptr)
        {
//              write(1, "Heap and stack collision\n", 25);
//              abort();
                errno = ENOMEM;
                return (caddr_t) -1;
        }

        heap_end += incr;

        return (caddr_t) prev_heap_end;
}

int _close_r(struct _reent *ptr, int file)
{
        return -1;
}

int _fstat_r(struct _reent *ptr, int file, struct stat *st)
{
        st->st_mode = S_IFCHR;
        return 0;
}
int _isatty_r(struct _reent *ptr, int file)
{
        return 1;
}
_off_t _lseek_r(struct _reent *ptr, int i, off_t j, int p)
{
        return 0;
}
