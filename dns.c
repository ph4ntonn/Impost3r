#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>

#include "dns.h"

const char *address_types[16] = 
{
  "A",
  "NS",
  "MD",
  "MF",
  "CNAME",
  "SOA",
  "MB",
  "MG",
  "MR",
  "NULL",
  "WKS",
  "PTR",
  "HINFO",
  "MINFO",
  "MX",
  "TXT"
};

const char *ddress_classes[4] = 
{
  "IN",
  "CS",
  "CH",
  "HS"
};

DNS_header *
create_header() 
{
  srandom(time(NULL));

  DNS_header *header = malloc(sizeof(DNS_header));
  memset(header,0,sizeof(DNS_header));

  header->id = random();
  header->flags |= htons(0x0100); 
  header->qdcount = htons(1);     
  
  return header;
}

DNS_question *
create_question(const char *hostname) 
{
  DNS_question *question = malloc(sizeof(DNS_question) + strlen(hostname) + 2);

  question->length = strlen(hostname) + 2;
  question->qtype  = htons(1);
  question->qclass = htons(1);

  char *token;
  const char delim[2] = ".";

  char *hostname_dup = strdup(hostname);
  token = strtok(hostname_dup, delim);

  char *q_qname = &question->qname[0];
  while(token != NULL) {
    size_t len = strlen(token);

    *q_qname = len;                   
    q_qname++;                        
    strncpy(q_qname, token, len + 1); 
    q_qname += len;                   

    token = strtok(NULL,delim);      
  }

  free(hostname_dup);
  return question;
}

size_t 
build_packet(DNS_header *header, DNS_question *question, unsigned char **packet) 
{
  size_t header_s = sizeof(DNS_header);
  size_t question_s = question->length + sizeof(question->qtype) + sizeof(question->qclass);
  size_t length = header_s + question_s;

  *packet = malloc(length);

  int offset = 0;
  memcpy(*packet + offset, header, sizeof(DNS_header));
  offset += sizeof(DNS_header);
  memcpy(*packet + offset, question->qname, question->length);
  offset += question->length;
  memcpy(*packet + offset, &question->qtype, sizeof(question->qtype)); 
  offset += sizeof(question->qtype);
  memcpy(*packet + offset, &question->qclass, sizeof(question->qclass)); 

  return length;
}
