#ifndef _UTIL_H
#define _UTIL_H

#define PATH 128
#define LISTENQ 1024
#define BUFSIZE 8192
#define DELIM "="

#define IAN_CONF_OK 0
#define IAN_CONF_ERROR -1

typedef struct ian_conf{
    char root[PATH];
    int port;
    int thd_num;
} ian_conf_t;

int read_conf(char *filename, ian_conf_t * conf);