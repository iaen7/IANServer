#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "http_request.h"

static int ian_http_process_ignore(ian_http_request_t *request, ian_http_out_t* out, char* data, int len);
static int ian_http_process_connection(ian_http_request_t* request, ian_http_out_t* out, char* data, int len);
static int ian_http_process_if_modified_since(ian_http_request_t* request, ian_http_out_t* out, char* data, int len);

ian_http_handle_header_t ian_http_headers_in[] = {
    {"Host", ian_http_process_ignore},
    {"Connection", ian_http_process_connection},
    {"If-Modified-Since", ian_http_process_if_modified_since},
    {"", ian_http_process_ignore}
};

int ian_http_process_ignore(ian_http_request_t *request, ian_http_out_t* out, char* data, int len){
    (void) request;
    (void) out;
    (void) data;
    (void) len;
    return 0;
}

//处理连接
int ian_http_process_connection(ian_http_request_t* request, ian_http_out_t* out, char* data, int len){
    (void) request;
    if(!strncmp("keep-alive", data, len)){
        out->keep_alive = 1;
    }
    return 0;
}

//处理是否被修改
int ian_http_process_if_modified_since(ian_http_request_t* request, ian_http_out_t* out, char* data, int len){
    (void) request;
    (void) len;
    struct tm tm;

    if(strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char*) NULL){
        return 0;
    }

    time_t client_time = mktime(&tm);

    if(fabs(difftime(client_time, out->mtime)) < 1e-6){
        out->modified = 0;
        out->status = IAN_HTTP_NOT_MODIFIED;
    }
    return 0;
}

int ian_init_request_t(ian_http_request_t* request, int fd, int epfd, char* path){
    request->root = path;
    request->fd = fd;
    request->epfd = epfd;
    request->pos = 0;
    request->last = 0;
    request->state = 0;
    INIT_LIST_HEAD(&(request->list));
    return 0;
}

int ian_init_out_t(ian_http_out_t* out, int fd){
    out->fd = fd;
    out->keep_alive = 1;
    out->modified = 1;
    out->status = 200;
    return 0;
}

void ian_http_handle_header(ian_http_request_t* request, ian_http_out_t *out){
    list_head_t* pos;
    ian_http_header_t* hd;
    ian_http_header_handle_t* head_in;
    int len;
    //后向遍历request里的请求头
    list_for_each(pos, &(request->list)){
        //找到其在header结构的位置
        hd = list_entry(pos, tk_http_header_t, list);
        for(header_in = ian_http_headers_in; strlen(header_in->name) > 0; header_in++){
            if(strncmp(hd->key_start, header_in->name, hd->key_end-hd->key_start) == 0)
                len = hd->value_end - hd->value_start;
                (*(header_in->handler))(request, out, hd->value_start, len);
                break;
        }
    }
    list_del(pos);
    free(hd);
}

//根据状态码返回shortmsg
const char* get_shortmsg_from_status_code(int status_code){
    if(status_code == IAN_HTTP_OK){
        return "OK";
    }
    if(status_code == IAN_HTTP_NOT_MODIFIED)
        return "Not Modified";
    if(status_code == IAN_HTTP_NOT_FOUND){
        return "Not Found";
    }
    return "Unknown";
}

int ian_http_close_conn(ian_http_request_t* request){
    close(request->fd);
    free(request);
    return 0;
}