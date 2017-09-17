#ifndef _HTTP_PARSE_H
#define _HTTP_PARSE_H

#include "http_request.h"
#include "list.h"

#define CR '\r'
#define LF '\n'

#define MAX_BUF 8142

#define IAN_HTTP_PARSE_INVALID_METHOD   10
#define IAN_HTTP_PARSE_INVALID_REQUEST  11
#define IAN_HTTP_PARSE_INVALID_HEADER   12

#define ian_str3_cmp(m, c0, c1, c2, c3)         \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define ian_str3Ocmp(m, c0, c1, c2, c3)         \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define ian_str4cmp(m, c0, c1, c2, c3)         \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

//http请求行解析
int ian_http_parse_request_line(ian_http_request_t *request);
//http请求体解析
int ian_http_parse_request_body(ian_http_request_t *request);

#endif