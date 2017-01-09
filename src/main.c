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

#include "./common.h"
#include "./stream/stream.h"
#include <phread.h>
#include <stdargs.h>

static	pthread_mutex_t			print_lock;

static	void	usage(const char* filename);
static	int		do_work();
static	void	thread_printf(char* fmt, ...);

int main(int argc, char* argv[])
{
    //Get args
    pthread_mutex_init(&print_lock, NULL);

    if(argc != 5) {
        usage(argv[0]);
        return -1;
    }

    pstream_t p_s1, p_s2;
    return 0;
}

void usage(const char* filename)
{
    printf("Usage:\n"
           "\t%s input-stream-type1 path1 input-stream-type2 path2\n"
           "Supported type:\n"
           "\tpty"
           "\tunix-spcket\n",
           filename);

    return;
}
