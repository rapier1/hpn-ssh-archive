
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/sysctl.h>
#include <string.h>
#include <estats/estats.h>
#include <pthread.h>

#ifndef WEB10G
#define WEB10G 1
#endif

#define Chk(x) \
    do { \
        err = (x); \
        if (err != NULL) { \
            goto Cleanup; \
        } \
    } while (0)

#define ChkIgn(x) \
    do { \
        err = (x); \
        if (err != NULL) { \
            estats_error_free(&err); \
            goto Cleanup; \
        } \
    } while (0)

#define SWAP(x, y) \
    do { \
        typeof(x) tmp; \
        tmp = x; \
        x = y; \
        y = tmp; \
    } while (0)

#define PRINT_AND_FREE(err) \
    do { \
        estats_error_print(stderr, err); \
        estats_error_free(&err); \
    } while (0)


// init web10g vars
estats_error* web10g_init();

// verify kernel is web10g
int web10g_check_kernel();

// verify that tcp_estats_nl module is loaded
int web10g_check_module();

// spawns a thread that writes to a file
void web10g_thread_file();

// return this process's cid
int web10g_get_cid();

// set the value of the web10g local cid variable
void web10g_set_cid(int);

// Given a process's PID, find its CID.
estats_error* web10g_find_cid(pid_t);

// writes a record based on the process's current web10g statistics
estats_error* web10g_writerecord();

// assign the value of parameter to be the web10g 'LimRwin' variable's value
void web10g_get_LimRwin(uint32_t*);

// assign the value of parameter to be the web10g 'OctetsRetrans' variable's value
void web10g_get_OctetsRetrans(uint32_t*);

// assign the value of parameter to be the web10g 'SampleRTT' variable's value
void web10g_get_SampleRTT(uint32_t*);

// set the value of the web10g 'LimRwin' variable to the value of the parameter
estats_error* web10g_set_limRwin(uint32_t*);

// free and destroy web10g vars
void web10g_free();

// spawns a thread that periodically invokes the web10g readvars command
void web10g_start_readvars();
