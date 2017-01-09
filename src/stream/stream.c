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

#include "./stream.h"
#include "./pty/pty.h"
#include "./unix-socket/unix-socket.h"

#define STREAM_TYPE_PTY				0
#define STREAM_TYPE_UNIX_SOCKET		1

static	stream_open_t		type_open_table[] = {
    [STREAM_TYPE_PTY] = pty_open,
    [STREAM_TYPE_UNIX_SOCKET] = unix_socket_open
};

pstream_t open_stream(const char* name, const char* path)
{
    u32 type;

    if(strcmp(name, "pty") == 0) {
        type = STREAM_TYPE_PTY;

    } else if(strcmp(name, "unix-socket") == 0) {
        type = STREAM_TYPE_UNIX_SOCKET;

    } else {
        return NULL;
    }

    return type_open_table[type](path);
}
