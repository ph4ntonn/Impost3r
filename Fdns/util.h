//
// Created by zz on 2015/6/20.
//

#ifndef FDNS_UTIL_H
#define FDNS_UTIL_H

#include <stdint.h>

#define MAX_NAME_SIZE 2048


/*
 * Dns request Entity
 */
typedef struct dns_request_query{
    char name[MAX_NAME_SIZE];
    uint16_t type;
    uint16_t class;
}dns_request_query;

typedef struct dns_request{

    uint16_t transaction_id;

    uint16_t flags;
    uint16_t questions;
    uint16_t answer_rrs;
    uint16_t authority_rrs;
    uint16_t additional_rrs;

    dns_request_query *queries;   //array of request queries

}dns_request;

/*
 * method
 * return 0 for succeed
 * return -1 for failed
 */

int resolve_dns_request(void *buf,int bufsize,dns_request *request);
int resolve_dns_request_query(void *buf,void **ptr,dns_request_query *query);
void free_dns_request(struct dns_request *dnsRequest);


/*
 * Dns response Entity
 */

//Answers
typedef struct dns_response_answer{
    uint16_t name_pointer;  //the pointer to the name
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t length;    //the data length of name .For address is 4
    union cname_or_address{
        char cname[MAX_NAME_SIZE];
        uint16_t address;
    };
}dns_response_answer;

typedef struct dns_response_autho_nameserver{
    union name{
        char address[MAX_NAME_SIZE];
        uint16_t name_pointer;
    };
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t length;    //the data length of name .For address is 4
}dns_response_autho_nameserver;

typedef struct dns_response{

    uint16_t transaction_id;

    uint16_t flags;
    uint16_t questions;
    uint16_t answer_rrs;
    uint16_t authority_rrs;
    uint16_t additional_rrs;

    dns_request_query *queries;   //array of request queries
    dns_response_answer *answers;   //array of response answers
}dns_response;

int setup_dns_response(char *buf,int *bufsize,const struct dns_request *request,struct dns_response *response);
void free_dns_respnose(struct dns_response *response);
#endif //FDNS_UTIL_H
