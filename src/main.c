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
#include <pthread.h>
#include <stdarg.h>
#include <signal.h>

static	pthread_mutex_t			print_lock;
static	pthread_mutex_t			exit_lock;
static	volatile bool			run_flag = false;
static  volatile pstream_t		p_s1;
static  volatile pstream_t		p_s2;
static	volatile pthread_t		thread1;
static	volatile pthread_t		thread2;

static	void	usage(const char* filename);
static	void	thread_printf(char* fmt, ...);
static	void	thread_print_buf(char* fmt, u8* buf, size_t len, ...);
static	void	do_work(pstream_t p_s1, pstream_t p_s2, bool _1_to_2);
static	void*	s2_send_thread(void* args);
static	void	do_exit(int);

int main(int argc, char* argv[])
{
    //Get args
    pthread_mutex_init(&print_lock, NULL);

    if(argc != 5) {
        usage(argv[0]);
        return -1;
    }

    const char* p_s1_type = argv[1];
    const char* path1 = argv[2];
    const char* p_s2_type = argv[3];
    const char* path2 = argv[4];

    //Open stream
    p_s1 = open_stream(p_s1_type, path1);

    if(p_s1 == NULL) {
        printf("Failed to open stream 1.\n");
        goto EXCEPT_OPEN_S1;
    }

    printf("Stream1 opened.\n");

    p_s2 = open_stream(p_s2_type, path2);

    if(p_s2 == NULL) {
        printf("Failed to open stream 2.\n");
        goto EXCEPT_OPEN_S2;
    }

    printf("Stream2 opened.\n");

    thread1 = pthread_self();

    //Create stream 2 send thread
    pthread_mutex_init(&exit_lock, NULL);
    run_flag = true;

    if(pthread_create((pthread_t* restrict)&thread2, NULL, s2_send_thread,
                      NULL) != 0) {
        printf("Failed to create thread.\n");
        goto EXCEPT_CREATE_THREAD;
    }

    thread_printf("Fake serial enabled.\n");

    //Stream 1 send thread
    do_work(p_s1, p_s2, true);

    pthread_join(thread2, NULL);
    //p_s2->close(p_s2);
    //p_s1->close(p_s1);
    printf("Exited.\n");

    return 0;

EXCEPT_CREATE_THREAD:
    p_s2->close(p_s2);

EXCEPT_OPEN_S2:
    p_s1->close(p_s1);

EXCEPT_OPEN_S1:
    return -1;
}

void usage(const char* filename)
{
    printf("Usage:\n"
           "\t%s input-stream-type1 path1 input-stream-type2 path2\n"
           "Supported type:\n"
           "\tpty\n"
           "\tunix-spcket\n",
           filename);

    return;
}

void thread_printf(char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    pthread_mutex_lock(&print_lock);
    vprintf(fmt, ap);
    pthread_mutex_unlock(&print_lock);
    return;
}

void thread_print_buf(char* fmt, u8 * buf, size_t len, ...)
{
    va_list ap;
    va_start(ap, len);
    pthread_mutex_lock(&print_lock);
    vprintf(fmt, ap);

    for(size_t i = 0; i < len; i++) {
        if(i % 16 == 0) {
            printf("\n0x%.4X :", i);
        }

        printf(" %.2X", (u32) * (buf + i));
    }

    printf("\n");

    pthread_mutex_unlock(&print_lock);
    return;
}

void do_work(pstream_t p_s1, pstream_t p_s2, bool _1_to_2)
{
    pstream_t p_src;
    pstream_t p_dest;

    signal(SIGINT, do_exit);

    if(_1_to_2) {
        p_src = p_s1;
        p_dest = p_s2;

    } else {
        p_src = p_s2;
        p_dest = p_s1;
    }

    while(run_flag) {
        u8 buf[1024];
        size_t read_len = p_src->read(p_src, buf, sizeof(buf));

        if(read_len == 0) {
            continue;
        }

        while(!p_dest->write(p_dest, buf, read_len)) {
            if(errno == EINTR) {
                if(!run_flag) {
                    break;
                }

            } else {
                break;
            }
        }

        if(!run_flag) {
            break;
        }

        char* fmt;

        if(_1_to_2) {
            fmt = "%d byte(s) sent from stream1 to stream2,";

        } else {
            fmt = "%d byte(s) sent from stream2 to stream1,";
        }

        thread_print_buf(fmt, buf, read_len, read_len);
    }

    return;
}

void do_exit(int unused)
{
    thread_printf("SIGINT caught.\n");

    run_flag = false;
    pthread_mutex_lock(&exit_lock);

    if(pthread_self() == thread1) {
        p_s1->close(p_s1);
        p_s1 = NULL;

    } else if(pthread_self() == thread2) {
        p_s2->close(p_s2);
        p_s2 = NULL;
    }

    if(p_s1 != NULL) {
        pthread_kill(thread1, SIGINT);
    }

    if(p_s2 != NULL) {
        pthread_kill(thread2, SIGINT);
    }

    pthread_mutex_unlock(&exit_lock);

    if(pthread_self() == thread2) {
        pthread_exit(NULL);

    } else {
        exit(0);
    }

    UNREFERENCED_PARAMETER(unused);
    return;
}

void* s2_send_thread(void* args)
{
    do_work(p_s1, p_s2, false);
    pthread_exit(NULL);
    return NULL;

    UNREFERENCED_PARAMETER(args);
}
