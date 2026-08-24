/* SPDX-License-Identifier: LicenseRef-STM32-CANopen-Research-Education-Commercial-1.0 */
/*
 * Minimal newlib syscall policy for this no-console, static-allocation reference.
 * The application has no runtime heap contract and no file-descriptor transport.
 * Replace these only when a board-specific diagnostic channel is deliberately
 * designed, reviewed, and assigned timing/error-handling requirements.
 */
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

int
_close(int file) {
    (void)file;
    errno = EBADF;
    return -1;
}

int
_fstat(int file, struct stat *st) {
    (void)file;
    if (st != NULL) {
        st->st_mode = S_IFCHR;
    }
    return 0;
}

int
_isatty(int file) {
    (void)file;
    return 1;
}

int
_kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int
_getpid(void) {
    return 1;
}

off_t
_lseek(int file, off_t offset, int whence) {
    (void)file;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return (off_t)-1;
}

int
_read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    errno = EBADF;
    return -1;
}

void *
_sbrk(ptrdiff_t increment) {
    (void)increment;
    errno = ENOMEM;
    return (void *)-1;
}

int
_write(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    errno = EBADF;
    return -1;
}
