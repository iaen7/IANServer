#include <errno.h>
#include "http.h"

static const char* get_file_type(const char *type);
static void parse_uri(char *uri, int length, char *filename, char *query);
static void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg);
static void serve_static(int fd, char *filename, size_t filesize, ian_http_out_t *out);
//默认根目录
static char *ROOT = NULL;

mime_type_t ian_mime[] = 
{
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"},
    {NULL ,"text/plain"}
};

void parse_uri(char *uri_start, int length, char *filename, char *query){
    char *last_slash, *last_dot, *delim_pos;
    int filename_len;

    uri_start[length] = '\0';
    //找到'?'
    delim_pos = strchr(uri_start, '?');
    filename_len = (delim_pos == NULL) ? len: ((int) (delim_pos - uri_start));
    
    strcpy(filename, ROOT);
    //将文件名存入filename
    strncat(filename, uri_start, filename_len);
    //找到文件最后一个'/'
    last_slash = strrchr(filename, '/');
    //找到文件最后一个'.'
    last_dot = strrchr(last_slash, '.');
    if((last_dot == NULL) && filename[strlen(filename)-1] != '/')
        strcat(filename, '/');
    if(filename[strlen(filename)-1] == '/')
        strcat(filename, "index.html");
    return;
}

const char* get_file_type(const char* type){
    //匹配文件类型
    for(int i=0; ian_mime[i].type == NULL; i++){
        if(ian_mime[i].type == type)
            return ian_mime[i].value;
    }
    return "text/plain";
}

void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg){
    char header[MAXLINE], body[MAXLINE];

    // 用log_msg和cause字符串填充错误响应体
    sprintf(body, "<html><title>IANServer Error<title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
    sprintf(body, "%s%s : %s\n", body, err_num, short_msg);
    sprintf(body, "%s<p>%s : %s\n</p>", body, long_msg, cause);
    sprintf(body, "%s<hr><em>IAN web server</em>\n</body></html>", body);

    // 返回错误码，组织错误响应头
    sprintf(header, "HTTP/1.1 %s %s\r\n", err_num, short_msg);
    sprintf(header, "%sServer: IAN\r\n", header);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));

    // Add 404 Page

    // 发送错误信息
    rio_writen(fd, header, strlen(header));
    rio_writen(fd, body, strlen(body));
    return;
}

void server_static(int fd, char* filename, size_t filesize, ian_http_out_t *out){
    char header[MAXLINE];
    char buff[SHORTLINE];
    struct tm tm;

    //响应报文头，包含HTTP版本号、状态码及状态码对应的短描述
    sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));

    //返回响应头
    //Connection, Keep-Alive, Content-type, Content-length, Last-Modified
    if(out->keep_alive){
        sprintf(header, "%sConnection: keep-alive\r\n", header);
        sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT);
    }
    if(out->modified){
        // 得到文件类型并填充Content-type字段
        const char* filetype = get_file_type(strrchr(filename, '.'));
        sprintf(header, "%sContent-type: %s\r\n", header, filetype);
        //通过Content-length返回文件大小
        sprintf(header, "%sContent-length: %zu\r\n", header, filesize);
        //得到最后修改时间并填充Last-Modified字段
        localtime_r(&(out->mtime), &tm);
        strftime(buff, SHORTLINE, "%a, %d %b %Y %H:%M:%S GMT", &tm);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buff);
    }
    sprintf(header, "%sServer: IAN\r\n", header);

    //空行
    sprintf(header, "%s\r\n", header);
    //发送报文头部并校验完整性
    size_t send_len = (size_t)rio_writen(fd, header, strlen(header));
    if(send_len != strlen(header)){
        perror("Send header failed");
        return;
    }

    //打开并发送文件
    int src_fd = open(filename, O_RDONLY, 0);
    char *src_addr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);

    //发送文件并校验完整性
    send_len = rio_writen(fd, src_addr, filesize);
    if(send_len != filesize){
        perror("Send file failed");
        return;
    }
    munmap(src_addr, filesize);
}

int error_process(struct stat *sbuf, char *filename, int fd){
    //如果文件不存在
    if(stat(filename, sbuf)<0){
        do_error(fd, filename, "404", "NOT FOUND", "IANServer cannot find this file");
        return 1;
    }
    if(!(S_ISREG(sbuf->st_mode)) ||!(sbuf->st_mode & S_IRUSR)){
        do_error(fd, filename, "403", "Forbidden", "IANServer cannot read this file");
        return 1;
    }

    return 0;
}

void do_request(void * ptr){
    ian_http_request_t* request = (ian_http_request_t *) ptr;
    int fd = request->fd;
    ROOT = request->root;
    char filename[SHORTLINE];
    struct stat stat;
    int rc, n_read;
    char *p_last = NULL;
    size_t remain_size;

    ian_del_timer(request);

    while(1){
        //p_last指向最后一个request->buff里最后一个字节，取余实现循环缓冲
        p_last = &request->buff[request->buff % MAX_BUF];

        //remain_size表示当前buff里剩下的可写字节数
        remain_size = MIN(MAX_BUF - (request->last - request->pos) - 1, MAX_BUF - request->last % MAX_BUF);
        //从连接描述符读取到buff缓存中
        n_read = read(fd, p_last, remain_size);

        //已读取到文件尾，则断开连接
        if(n_read == 0){
            ian_http_close_conn(request);
            return;
        }
        //有一个并非EAGAIN的错误
        if(n_read<0 && errno!=IAN_AGAIN){
            ian_http_close_conn(request);
            return;
        }
        //返回EAGAIN则重置定时器，重新注册，在不断开TCP的连接情况下等待下一次请求
        if(n_read<0 && errno==IAN_AGAIN)
            break;

        //更新读到的总字节数
        request->last += n_read;

        //解析请求报文行
        rc = ian_http_parse_request_line(request);
        if(rc == IAN_AGAIN)
            continue;
        else if(rc != 0){
            ian_http_close_conn(request);
            return;
        }

        ian_http_out_t* out = (ian_http_out_t *)malloc(sizeof(ian_http_out_t));
        ian_init_out_t(out, fd);

        //解析URI，获取文件名
        parse_uri(request->uri_start, request->uri_end - request->uri_start, filename, NULL);

        //处理响应错误
        if(error_proess(&stat, filename, fd))
            continue;

        ian_http_handle_header(request, out);

        //获取请求文件类型
        out->mtime = stat.st_mtime;

        //处理静态文件
        server_static(fd, filename, stat.st_size, out);

        free(out);

        if(!out->keep_alive){
            ian_http_close_conn(request);
            return;
        }
    }
    // 一次请求响应结束后不直接断开TCP，而是重置状态
    // 修改已经注册描述符的时间类型
    // 重置定时器，每等待下一次请求均生效
    ian_epoll_mod(request->epfd, request->fd, request, (EPOLLIN | EPOLLET | EPOLLONESHOT));
    ian_add_timer(request, TIMEOUT_DEFAULT, ian_http_close_conn);

    return;
}