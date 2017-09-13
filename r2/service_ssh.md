## SSH服务安全
SSH服务是目前Linux操作系统上最常用的远程登录服务，其安全性对Linux系统安全的重要性不言而喻。
<br>
![openssh-logo](images/openssh-logo.gif)
### 改变默认端口
SSH服务配置文件一般位于`/etc/ssh/sshd_config`，本文若无特殊说明均在该文件中进行的参数配置。
<br>
虽然端口扫描工具(eg. nmap)可以根据端口的特征判断出具体的服务，但改变SSH服务的默认端口(22)可以预防一些低级攻击者使用默认端口对系统进行攻击。
<br>
编辑`/etc/ssh/sshd_config`文件，设置:
```
Port 7536    #或规划一个其他端口
```
必须重启ssh服务才能生效，下同。
### 绑定监听地址
SSH服务默认监听本机全部接口(`ListenAddress 0.0.0.0`)，需要根据业务场景将其绑定到具体的接口上，避免全网监听，降低SSH服务被攻击的可能性。
<br>
例如，一个对外提供WEB服务的服务器，仅需要SSH服务监听管理平面即可。
```
#ListenAddress 0.0.0.0    #注释掉，禁止全网监听
ListenAddress 192.168.1.10
ListenAddress 192.168.2.10
```
### 记录日志
```
SyslogFacility	AUTH    #日志分类，也可设置为AUTHPRIV
LogLevel	INFO    #日志级别，也可设置为VERBOSE
```
### 设置认证方式
```
ChallengeResponseAuthentication	yes    #启用Challenge-Response认证方式
UsePAM yes    #启用PAM认证
PubkeyAuthentication	yes    #启用公私钥认证方式
HostbasedAuthentication	no    #禁止基于主机的认证方式
PasswordAuthentication	no    #禁止password认证
```
### 使用安全的算法
```
Ciphers	aes128-ctr,aes192-ctr,aes256-ctr    #设置消息加密算法
MACs	hmac-sha2-256,hmac-sha2-512    #设置消息校验算法
```
### 禁止root直接ssh登录系统
```
PermitRootLogin no
```
如果业务需要root直接ssh登录，需使用Pubkey认证，禁止root使用口令认证:
```
PermitRootLogin without-password
```
### 用户访问控制
以下4个参数可以实现对用户的SSH服务访问控制，允许或禁止指定的用户/用户组SSH登录系统。
```
#Allow白名单
AllowUsers	user1 user2
AllowGroups	group1 group2

#Deny黑名单
DenyUsers	user3 user4
DenyGroups	group3 group4
```
推荐使用白名单(AllowUsers/AllowGroups)进行访问控制:
```
NKG1000115469:~ # grep AllowGroups /etc/ssh/sshd_config
AllowGroups sshusers    #设置仅允许sshusers组内的用户ssh登录系统
NKG1000115469:~ # 
NKG1000115469:~ # groupadd sshusers    #创建sshusers组
NKG1000115469:~ # useradd -G sshusers tom    #创建用户时加入sshusers组
NKG1000115469:~ # usermod -a -G sshusers jack    #修改用户，加入sshusers组
```
### 主机访问控制
通过tcp wrapper对SSH服务进行主机访问控制:
```
# IP白名单
NKG1000115469:~ # cat /etc/hosts.allow
sshd: 192.168.1.0/24, 192.168.2.0/24 : ALLOW

# IP黑名单
NKG1000115469:~ # cat /etc/hosts.deny
sshd: ALL : DENY
```
### 空闲超时退出
以下设置空闲300秒，则断开连接:
```
ClientAliveCountMax	300
ClientAliveInterval	0
```
### sftp用户Chroot配置
```
Match User sftpusr
	ChrootDirectory /chroot    #目录的属主必须是root
	ForceCommand interal-sftp -f AUTH -l INFO
```
### 其他参数配置
```
Protocol 2    #使用SSHv2版本
AllowAgentForwarding no    #禁止代理转发
AllowTcpForwarding no	#禁止TCP转发
Banner /etc/issue.net
GatewayPorts no
LoginGraceTime 60
MaxAuthTries 4	#设置单次连接认证次数，防范暴力破解
PermitEmptyPasswords no    #禁止空口令认证
IgnoreRhosts yes
IgnoreUserKnownHosts yes
RhostsRSAAuthentication no
StrictModes yes
Subsystem sftp interal-sftp -f AUTH -l INFO    #设置sftp记录日志
UsePrivilegeSeparation yes
X11Forwarding no
PermitTunnel no
PermitUserEnvironment no
PrintLastLog yes    #打印上次登录信息
PrintMotd yes
```
更多设置，参考`man sshd_config`。
### 防范暴力破解
利用PAM模块pam_tally2进行防范暴力破解，编辑/etc/pam.d/sshd文件，添加：
```
#用户认证失败30次被锁定，900秒后自动解锁
auth required pam_tally2.so onerr=fail audit silent deny=30 unlock_time=900
account required pam_tally2.so
```
上面配置pam_tally2模块是针对系统上所有的用户，但用户被锁定可能对业务正常流程有影响，可以参考[pam_tally2_custom](pam.md#pam_tally2_custom)设置例外用户。
<br>
其他防范暴力破解的工具:
<br>
[denyhosts](https://github.com/denyhosts/denyhosts)
<br>
[fail2ban](https://github.com/fail2ban/fail2ban)
### 隐藏OpenSSH版本
隐藏或伪造OpenSSH版本可以迷惑攻击者，防止根据版本信息获取已知安全漏洞，增加攻击难度。
```
NKG1000115469:~ # nmap -sV 127.0.0.1 -p22
PORT   STATE SERVICE VERSION
22/tcp open  ssh     OpenSSH 7.2 (protocol 2.0)    //使用nmap进行端口扫描可以看到使用的OpenSSH版本为7.2
NKG1000115469:~ #
NKG1000115469:~ # cp -p /usr/sbin/sshd /usr/sbin/sshd.bak
NKG1000115469:~ # sed -i "s/OpenSSH_7.2/OpenSSH_X.X/g" /usr/sbin/sshd    //将版本信息替换掉
NKG1000115469:~ # systemctl restart sshd
NKG1000115469:~ # 
NKG1000115469:~ # nmap -sV 127.0.0.1 -p22
PORT   STATE SERVICE VERSION
22/tcp open  ssh     OpenSSH X.X (protocol 2.0)   //替换后，看不到OpenSSH版本
NKG1000115469:~ # 
```
