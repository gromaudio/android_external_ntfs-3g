/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU LGPLv2.
    See the file COPYING.LIB.
*/

#include "config.h"
#include "fuse_i.h"
#include "fuse_lowlevel.h"

struct fuse_chan *fuse_mount(const char *mountpoint, struct fuse_args *args)
{
    struct fuse_chan *ch;
    int fd;

    fd = fuse_kern_mount(mountpoint, args);
    if (fd == -1)
        return NULL;

    ch = fuse_kern_chan_new(fd);
    if (!ch)
        fuse_kern_unmount(mountpoint, fd);

    return ch;
}

void fuse_unmount(const char *mountpoint, struct fuse_chan *ch)
{
    int fd = ch ? fuse_chan_fd(ch) : -1;
    fuse_kern_unmount(mountpoint, fd);
    fuse_chan_destroy(ch);
}

int fuse_version(void)
{
    return FUSE_VERSION;
}

int fuse_daemonize(int foreground)
{
    if (!foreground) {
        int nullfd;

        /*
         * demonize current process by forking it and killing the
         * parent.  This makes current process as a child of 'init'.
         */
        switch(fork()) {
        case -1:
            perror("fuse_daemonize: fork");
            return -1;
        case 0:
            break;
        default:
            _exit(0);
        }

        if (setsid() == -1) {
            perror("fuse_daemonize: setsid");
            return -1;
        }

        (void) chdir("/");

        nullfd = open("/dev/null", O_RDWR, 0);
        if (nullfd != -1) {
            (void) dup2(nullfd, 0);
            (void) dup2(nullfd, 1);
            (void) dup2(nullfd, 2);
            if (nullfd > 2)
                close(nullfd);
        }
    }
    return 0;
}
