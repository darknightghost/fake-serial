/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./pty.h"
#include <pty.h>
#include <sys/select.h>

static	size_t	pty_read(ppty_stream_t p_this, u8* buf, size_t size);
static	bool	pty_write(ppty_stream_t p_this, const u8* buf, size_t size);
static	void	pty_close(ppty_stream_t p_this);

pstream_t pty_open(const char* path)
{
    //Allocate memory
    ppty_stream_t p_ret = malloc(sizeof(pty_stream_t));

    if(p_ret == NULL) {
        goto EXPECT_ALLOC_MEM;
    }

    p_ret->path = malloc(strlen(path) + 1);

    if(p_ret->path == NULL) {
        goto EXCEPT_ALLOC_PATH;
    }

    strcpy(p_ret->path, path);

    //Set pty attribute
    struct termios termo = {
        .c_iflag = IGNBRK | IGNPAR | IGNCR,
        .c_oflag = 0,
        .c_cflag = CLOCAL | CREAD,
        .c_lflag = 0
    };
    memset(&(termo.c_cc), 0, sizeof(termo.c_cc));

    //Open pty
    int fd_master, fd_slave;
    char slave_path[PATH_MAX];

    if(openpty(&fd_master, &fd_slave, slave_path, &termo, NULL) != 0) {
        printf("Failed to open pty.\n");
        goto EXCEPT_OPEN_PTY;
    }

    close(fd_slave);

    //Create link
    printf("Linking \"%s\" to \"%s\"...\n",
           slave_path,
           path);

    unlink(path);

    if(symlink(slave_path, path) != 0) {
        printf("Link failed.\n");
        goto EXCEPT_CREATE_LINK;
    }

    printf("Link created.\n");

    p_ret->fd_master = fd_master;
    p_ret->stream.read = (stream_read_t)pty_read;
    p_ret->stream.write = (stream_write_t)pty_write;
    p_ret->stream.close = (stream_close_t)pty_close;
    p_ret->stream.type = "pty";

    return (pstream_t)p_ret;

EXCEPT_CREATE_LINK:
    close(fd_master);

EXCEPT_OPEN_PTY:
    free(p_ret->path);

EXCEPT_ALLOC_PATH:
    free(p_ret);

EXPECT_ALLOC_MEM:
    return NULL;
}

size_t pty_read(ppty_stream_t p_this, u8* buf, size_t size)
{
    ssize_t read_len = read(p_this->fd_master, buf, size);

    if(read_len < 0) {
        if(errno == EIO) {
            usleep(500);
        }

        return 0;
    }

    return (size_t)read_len;
}

bool pty_write(ppty_stream_t p_this, const u8* buf, size_t size)
{
    const u8* p = buf;

    for(size_t count = 0; count < size;) {
        ssize_t write_len = write(p_this->fd_master,
                                  p, size - count);

        if(write_len < 0) {
            return false;
        }

        p += write_len;
        count += write_len;
    }

    return true;
}

void pty_close(ppty_stream_t p_this)
{
    close(p_this->fd_master);
    unlink(p_this->path);
    free(p_this->path);
    free(p_this);
    return;
}
