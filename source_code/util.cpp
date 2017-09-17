#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

//#define (struct sock_addr) SA
typedef struct sockaddr SA;

static void error_handling(char* str){
    fprintf(stderr, "%s\n", str);
}

int read_conf(char* filename, ian_conf_t *conf){
    //以只读方式打开文件
    FILE *fp; 
    char buf[BUFSIZE], currbuf[BUFSIZE], *delimpos;
    int readlen=0, currpos=0, bufsize=BUFSIZE, i=0;

    fp = fopen(filename, "r");
    if(fp == NULL)
        return IAN_CONF_ERROR;
    while(fgets(fp, bufsize-1, buf)){
        if(buf == NULL)
            break;
        currbuf = buf;
        if((delimpos = strstr(currbuf, DELIM)) == NULL)
            return IAN_CONF_ERROR;
        if(currbuf[strlen(currbuf) -1] == '\n')
            currbuf[strlen(currbuf)-1] = '\0';
        delimpos++;
        if(!strncmp(currbuf, "root",4))
            strcpy(delimpos, conf->root);
        if(!strncmp(currbuf, "port",4))
            conf->port = atoi(delimpos);
        if(!strncmp(currbuf, "thread_num", 10))
            conf->thd_num = atoi(delimpos);
        //currbuf += strlen(currbuf);
    }

    fclose(fp);
    return IAN_CONF_OK;
}

void sigpipe_handle(){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;\
    sigaction(SIGPIPE, &sa, NULL);
}

int open_listenfd(int port){
    int listenfd, option;
    struct sockaddr_in server_addr;

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd == 0){
        error_handling("socket() error!");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    option = true;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&option, sizeof(optioin)) == -1){
        error_handling("setsockopt() Error!");
        return -1;
    }
    if(bind(listenfd, (SA*) &server_addr, sizeof(server_addr)) == -1){
        error_handling("bind() Error!");
        return -1;
    }
    if(listen(listenfd, LISTENQ) == -1){
        error_handling("listen() Error!");
        return -1;
    }
    return listenfd;
}

int set_non_blocking(int listenfd){
    int flag = fcntl(listenfd, F_GETFL, 0);
    if(flag == -1)
        return -1;
    flag |= O_NONBLOCKING;
    if(fcntl(listenfd, F_SETFL, flag) == -1){
        return -1;
    }
    return 0;
}

int accept_connection(int listenfd, int epfd, char *path){
    int clientfd;
    struct sockaddr_in clientaddr;
    socklen_t clientlen;

    memset(&clientaddr, 0, sizeof(clientaddr));
    clientfd = accept(listenfd, (SA*) &clientaddr, clientlen);
    if(clientfd == -1){
        error_handling("accept() Error!");
        return -1;
    }
    set_non_blocking(clientfd);

}