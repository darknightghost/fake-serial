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

#include "./tty.h"
#include <pty.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

static	size_t	tty_read(ptty_stream_t p_this, u8* buf, size_t size);
static	bool	tty_write(ptty_stream_t p_this, const u8* buf, size_t size);
static	void	tty_close(ptty_stream_t p_this);

pstream_t tty_open(const char* path)
{
    //Allocate memory
    ptty_stream_t p_ret = malloc(sizeof(tty_stream_t));

    if(p_ret == NULL) {
        goto EXPECT_ALLOC_MEM;
    }

    p_ret->path = malloc(strlen(path) + 1);

    if(p_ret->path == NULL) {
        goto EXCEPT_ALLOC_PATH;
    }

    strcpy(p_ret->path, path);

    //Set tty attribute
    struct termios termo = {
        .c_iflag = IGNBRK | IGNPAR | IGNCR,
        .c_oflag = 0,
        .c_cflag = CLOCAL | CREAD,
        .c_lflag = 0
    };
    memset(&(termo.c_cc), 0, sizeof(termo.c_cc));

    //Open tty
    int fd;

    fd = open(path, O_RDWR);

    if(fd < 1) {
        printf("Failed to open tty.\n");
        goto EXCEPT_OPEN_TTY;
    }

    tcsetattr(fd, TCSANOW, &termo);

    p_ret->fd = fd;
    p_ret->stream.read = (stream_read_t)tty_read;
    p_ret->stream.write = (stream_write_t)tty_write;
    p_ret->stream.close = (stream_close_t)tty_close;
    p_ret->stream.type = "tty";

    return (pstream_t)p_ret;

EXCEPT_OPEN_TTY:
    free(p_ret->path);

EXCEPT_ALLOC_PATH:
    free(p_ret);

EXPECT_ALLOC_MEM:
    return NULL;
}

size_t tty_read(ptty_stream_t p_this, u8* buf, size_t size)
{
    ssize_t read_len = read(p_this->fd, buf, size);

    if(read_len < 0) {
        if(errno == EIO) {
            usleep(500);
        }

        return 0;
    }

    return (size_t)read_len;
}

bool tty_write(ptty_stream_t p_this, const u8* buf, size_t size)
{
    const u8* p = buf;

    for(size_t count = 0; count < size;) {
        ssize_t write_len = write(p_this->fd,
                                  p, size - count);

        if(write_len < 0) {
            return false;
        }

        p += write_len;
        count += write_len;
    }

    fsync(p_this->fd);

    return true;
}

void tty_close(ptty_stream_t p_this)
{
    close(p_this->fd);
    free(p_this->path);
    free(p_this);
    return;
}
