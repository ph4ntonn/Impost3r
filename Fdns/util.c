//
// Created by zz on 2015/6/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
//#include <winsock2.h>
#include "util.h"

#define MAX_NAME_SIZE 2048

int resolve_dns_request(void *buf,int bufsize,dns_request *request){

    int status;
    int i;
    uint16_t *ptr=buf;

    printf("[buffer size]%d\n",bufsize);

    if(bufsize<=12){
        fprintf(stderr,"[ERROR] bufsize is too short.\n");
        return -1;
    }

    request->transaction_id=ntohs(*ptr++);
    request->flags=ntohs(*ptr++);
    request->questions=ntohs(*ptr++);
    request->answer_rrs=ntohs(*ptr++);
    request->authority_rrs=ntohs(*ptr++);
    request->additional_rrs=ntohs(*ptr++);

    request->queries=malloc(request->questions*sizeof(struct dns_request_query));
    if(request->queries==NULL){
        fprintf(stderr,"[ERROR] malloc error.\n");
    }

    ///debug
    fprintf(stdout,"-----------start a request show-------------\n");
    fprintf(stdout,"[debug]Request transaction_id:%x\n",request->transaction_id);
    fprintf(stdout,"[debug]Request flags:%x\n",request->flags);
    fprintf(stdout,"[debug]Request questions:%x\n",request->questions);
    fprintf(stdout,"[debug]Request answer_rrs:%x\n",request->answer_rrs);
    fprintf(stdout,"[debug]Request authority_rrs:%x\n",request->authority_rrs);
    fprintf(stdout,"[debug]Request additional_rrs:%x\n",request->additional_rrs);

    ///undebug
    for(i=0;i<request->questions;i++){
        if(resolve_dns_request_query(buf, (void **) &ptr,&request->queries[i])!=0){
            fprintf(stderr,"[ERROR]Failed to resolve dns request.\n");
        }
    }
    fprintf(stdout,"-----------end a request show-------------\n");

}

int resolve_dns_request_query(void *buf,void **ptr,dns_request_query *query){
    char name[MAX_NAME_SIZE];
    int i;
    int sublen,len=0;
    void *tempptr=*ptr;
    sublen=*((uint8_t *)tempptr++);
    while(sublen!=0){
        if(len!=0){
            name[len++]='.';
        }
        for(i=0;i<sublen;i++){
           name[len++]=*((uint8_t *)tempptr++);
        }
        sublen=*((uint8_t *)tempptr++);
    }
    name[len]='\0';
    strcpy(query->name,name);
    query->type=ntohs(*((uint16_t *)tempptr++));
    query->class=ntohs(*((uint16_t *)tempptr++));
    /*
     * debug
     */
    fprintf(stdout,"[debug]Request url:%s\n",query->name);
    fprintf(stdout,"[debug]Request type:%x\n",query->type);
    fprintf(stdout,"[debug]Request class:%x\n",query->class);


    *ptr=tempptr;
    return 0;
}

void free_dns_request(struct dns_request *dnsRequest){
    free(dnsRequest->queries);
}

int setup_dns_response(char *buf,int *bufsize,const struct dns_request *request,struct dns_response *response){


    int size=0;
    //make a response
    response->transaction_id=request->transaction_id;
    response->flags=0x8180;  //standard query response ,no error
    response->questions=request->questions;
    response->answer_rrs=0x0001;
    response->authority_rrs=0x0000;
    response->additional_rrs=0x0000;

    response->queries=request->queries;

    response->answers=malloc(1*sizeof(struct dns_response_answer));
    char *bufptr=buf;

    *(uint16_t *)bufptr=htons(response->transaction_id);bufptr+=2;
    *(uint16_t *)bufptr=htons(response->flags);bufptr+=2;
    *(uint16_t *)bufptr=htons(response->questions);bufptr+=2;
    *(uint16_t *)bufptr=htons(response->answer_rrs);bufptr+=2;
    *(uint16_t *)bufptr=htons(response->authority_rrs);bufptr+=2;
    *(uint16_t *)bufptr=htons(response->additional_rrs);bufptr+=2;

    size+=12;

    int i;
    int sublen=0;
    char *subptr=request->queries[0].name;
    char *tempptr=request->queries[0].name;
    while(*subptr!='\0'){
        sublen=0;
        while(*subptr!='.'&&*subptr!='\0'){
            sublen++;
            subptr++;
        }
        *bufptr++= (char) sublen;
        for(i=0;i<sublen;i++){
            *bufptr++=*tempptr++;
        }
        size+=(sublen+1);
        if(*subptr=='\0'){
            *bufptr++=0;size++;
            break;
        }
        subptr++;
        tempptr++;
    }
    *(uint16_t *)bufptr=htons(0x0001);bufptr+=2;
    *(uint16_t *)bufptr=htons(0x0001);bufptr+=2;
    size+=4;


    //answers

    //name ptr
    struct sockaddr_in s;
    *(uint16_t *)bufptr=htons(0xc00c);bufptr+=2;
    *(uint16_t *)bufptr=htons(0x0001);bufptr+=2; //type A
    *(uint16_t *)bufptr=htons(0x0001);bufptr+=2; //class IN
    *(uint32_t *)bufptr=htonl(120);bufptr+=4; //TTL
    *(uint16_t *)bufptr=htons(0x0004);bufptr+=2; //data len
    *(uint32_t *)bufptr=htonl(0x7f000001);bufptr+=4; //ip address
    size+=16;
    *bufsize=size;

    return 0;
}

void free_dns_respnose(struct dns_response *response){
    free(response->answers);
}