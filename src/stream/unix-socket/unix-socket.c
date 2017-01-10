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

#include "./unix-socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_HEAD		"listen:"
#define	CONNECT_HEAD	"connect:"

#define	LISTEN		0
#define	CONNECT		1

static	size_t	unix_socket_read(punix_socket_stream_t p_this,
                                 u8* buf, size_t size);
static	bool	unix_socket_write(punix_socket_stream_t p_this,
                                  const u8* buf, size_t size);
static	void	unix_socket_close(punix_socket_stream_t p_this);
static	int		do_listen(const char* path, punix_socket_stream_t p_stream);
static	int		do_connect(const char* path, punix_socket_stream_t p_stream);

pstream_t	unix_socket_open(const char* path)
{
    //Allocate memory
    punix_socket_stream_t p_ret = malloc(sizeof(unix_socket_stream_t));

    if(p_ret == NULL) {
        goto EXPECT_ALLOC_MEM;
    }

    p_ret->path = malloc(strlen(path) + 1);

    if(p_ret->path == NULL) {
        goto EXCEPT_ALLOC_PATH;
    }

    //Connect to socket
    if(strncmp(LISTEN_HEAD , path, strlen(LISTEN_HEAD)) == 0) {
        if(do_listen(path + strlen(LISTEN_HEAD), p_ret) < 0) {
            goto EXCEPT_CONNECTION;
        }

    } else if(strncmp(CONNECT_HEAD , path, strlen(CONNECT_HEAD)) == 0) {
        if(do_connect(path + strlen(CONNECT_HEAD), p_ret) < 0) {
            goto EXCEPT_CONNECTION;
        }

    } else {
        goto EXCEPT_CONNECTION;
    }

    p_ret->stream.type = "unix-socket";
    p_ret->stream.read = (stream_read_t)unix_socket_read;
    p_ret->stream.write = (stream_write_t)unix_socket_write;
    p_ret->stream.close = (stream_close_t)unix_socket_close;

    return (pstream_t)p_ret;

EXCEPT_CONNECTION:
EXCEPT_ALLOC_PATH:
    free(p_ret);

EXPECT_ALLOC_MEM:
    return NULL;
}


size_t unix_socket_read(punix_socket_stream_t p_this, u8* buf, size_t size)
{
    ssize_t read_len = read(p_this->fd, buf, size);

    if(read_len < 0) {
        return 0;
    }

    return (size_t)read_len;
}

bool unix_socket_write(punix_socket_stream_t p_this, const u8 * buf, size_t size)
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

    return true;
}

void unix_socket_close(punix_socket_stream_t p_this)
{
    close(p_this->fd);

    if(p_this->mode == LISTEN) {
        unlink(p_this->path);
    }

    free(p_this->path);
    free(p_this);
    return;
}

int do_listen(const char* path, punix_socket_stream_t p_stream)
{
    p_stream->mode = LISTEN;
    strcpy(p_stream->path, path);
    unlink(path);

    //Create listen socket
    int listen_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if(listen_socket < 0) {
        printf("Failed to create socket.\n");
        goto EXCEPT_SOCKET;
    }

    //Bind
    struct sockaddr_un server_address;

    server_address.sun_family = AF_UNIX;

    strcpy(server_address.sun_path, path);

    if(bind(listen_socket, (struct sockaddr *)&server_address,
            sizeof(server_address)) < 0) {
        printf("Failed to bind socket.\n");
        goto EXCEPT_BIND;
    }

    //Listen
    printf("Listening...\n");

    if(listen(listen_socket, 1) < 0) {
        printf("Listen failed.\n");
        goto EXCEPT_LISTEN;
    }

    //Accept
    size_t client_len;
    printf("Accepting...\n");
    int fd = accept(listen_socket, (struct sockaddr *)&server_address,
                    (socklen_t *)&client_len);

    if(fd < 0) {
        printf("Failed to accept.\n");
        goto EXCEPT_LISTEN;
    }

    printf("Accepted.\n");
    close(listen_socket);
    p_stream->fd = fd;

    return 0;

EXCEPT_LISTEN:
EXCEPT_BIND:
    close(listen_socket);

EXCEPT_SOCKET:
    return -1;
}

int do_connect(const char* path, punix_socket_stream_t p_stream)
{
    p_stream->mode = CONNECT;
    strcpy(p_stream->path, path);

    //Create socket
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if(fd < 0) {
        printf("Failed to create socket.\n");
        goto EXCEPT_SOCKET;
    }

    //Connect
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);

    if(connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Failed to connect to \"%s\".\n", path);
        goto EXCEPT_CONNECT;
    }

    p_stream->fd = fd;

    return 0;

EXCEPT_CONNECT:
    close(fd);

EXCEPT_SOCKET:
    return -1;
}
