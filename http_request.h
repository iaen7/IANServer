#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "list.h"

#define IAN_AGAIN EAGAIN

#define IAN_HTTP_PARSE_INVALID_METHOD   10
#define IAN_HTTP_PARSE_INVALID_REQUEST  11
#define IAN_HTTP_PARSE_INVALID_HEADER   12

#define IAN_HTTP_UNKNOWN    0x0001
#define IAN_HTTP_GET        0x0002
#define IAN_HTTP_HEAD       0x0004
#define IAN_HTTP_POST       0x0008

#define IAN_HTTP_OK             200
#define IAN_HTTP_NOT_MODIFIED   304
#define IAN_HTTP_NOT_FOUND      404

#define MAX_BUF 8142
typedef struct ian_http_request{
    char* root;
    int fd;
    int epfd;
    char buff[MAX_BUF];
    size_t pos;
    size_t last;
    int state;

    void* request_start;
    void* method_end;
    int method;
    void* uri_start;
    void* uri_end;
    void* path_start;
    void* path_end;
    void* query_start;
    void* query_end;
    int http_major;
    int http_minor;
    void* request_end;

    list_head_t list;   //将请求头储存到链表中

    void* cur_header_key_start;
    void* cur_header_key_end;
    void* cur_header_value_start;
    void* cur_header_value_end;
    void* timer;
}ian_http_request_t;

typedef struct ian_http_out{
    int fd;
    int keep_alive;
    time_t mtime;
    int modified;
    int status;
}ian_http_out_t;

typedef struct ian_http_header{
    void* key_start;
    void* key_end;
    void* value_start;
    void* value_end;
    struct list_head list;
}ian_http_header_t;

typedef int (*ian_http_header_handler_func)(ian_http_request_t* request, 
            ian_http_out_t* out, char* data, int len);

typedef struct ian_http_header_handle{
    char *name;
    ian_http_header_handler_func handler;
} ian_http_header_handle_t;

extern ian_http_header_handle_t ian_http_headers_in[];

void ian_http_handle_header(ian_http_request_t* request, ian_http_out_t* out);
int ian_http_close_conn(ian_http_request_t* request);
int ian_init_request_t(ian_http_request_t* request, int fd, int epfd, char* path);
int ian_init_out_t(ian_http_out_t* out, int fd);
const char* get_shortmsg_from_status_code(int status_code);

#endif