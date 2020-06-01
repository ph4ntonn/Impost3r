/* Dns Server

Auothor:zz*/

#include <stdio.h>
///*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
//#include <winsock2.h>

//*/

#include "util.h"



#define MAX_BUFFER_SIZE 2048

void sys_error(const char *status){
    perror(status);
    exit(EXIT_FAILURE);
}

int main(int argc,char *argv[]){

    int sockfd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char buf[MAX_BUFFER_SIZE];

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0)
        sys_error("socket");

    bzero(&server_addr,sizeof(struct sockaddr_in));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(53);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(sockfd, (const struct sockaddr *) &server_addr,sizeof(struct sockaddr_in))<0)
        sys_error("bind");

    while(1){
        int result=0;
        int addrlen=sizeof(struct sockaddr_in);
        bzero(&client_addr,sizeof(struct sockaddr_in));
        result=recvfrom(sockfd,buf,MAX_BUFFER_SIZE,0, (struct sockaddr *) &client_addr,&addrlen);
        if(result<0)
            sys_error("recvfrom");
        fprintf(stdout,"[debug]Get an request from [%s] success.\n",inet_ntoa(client_addr.sin_addr));
        struct dns_request request;

        resolve_dns_request(buf,result,&request);

        //TODO give a response to client
        char response_buf[MAX_BUFFER_SIZE];
        int response_buf_size;
        struct dns_response response;
        setup_dns_response(response_buf,&response_buf_size,&request,&response);
        result=sendto(sockfd,response_buf,response_buf_size,0, (const struct sockaddr *) &client_addr,sizeof(struct sockaddr_in));
        if(result<0)
            sys_error("sendto");
        free_dns_request(&request);
        free_dns_respnose(&response);
    }

    printf("Test Framework.\n");
    return 0;
}

