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

#pragma once

#include "../common.h"

struct	_stream_t;

//size_t read(pstream_t p_this, u8* buf, size_t size);
typedef	size_t	(*stream_read_t)(struct _stream_t*, u8*, size_t);

//bool write(pstream_t p_this, const u8* buf, size_t size);
typedef	bool	(*stream_write_t)(struct _stream_t*, const u8*, size_t);

//void close(pstream_t p_this);
typedef void	(*stream_close_t)(struct _stream_t*);

typedef	struct	_stream_t {
    const char*		type;
    stream_read_t	read;
    stream_write_t	write;
    stream_close_t	close;
} stream_t, *pstream_t;

//pstream_t	stream_open(const char* path);
typedef	pstream_t	(*stream_open_t)(const char*);

pstream_t	open_stream(const char* name, const char* path);
