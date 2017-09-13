## 用户&组管理
用户是操作系统的使用主体，合理管理用户可防止用户被恶意利用，降低系统被攻击的风险。
<br>
![user-logo](images/user-logo.png)
### 合理规划用户&组
用户和用户组是Linux操作系统上进行权限控制的关键因素，合理规划用户和用户组对系统文件权限访问控制有重要意义。
<br>
先规划用户组，如sshusers、ftpusers等:
```
root@sles:~ # groupadd sshusers
root@sles:~ # groupadd ftpusers
root@sles:~ # 
```
在规划用户，程序运行账号、运维管理账号等，判断好这些用户是否要登录系统，要加入哪些用户组:
```
root@sles:~ # useradd -s /sbin/nologin apprunner    #程序运行账号不用登录系统
root@sles:~ # useradd -G sshusers appmanager    #运维管理账号
```
### 禁用系统功能账号
Linux操作系统默认安装完，会存在一些不会用到的系统功能账号，需要将其禁用掉，防止因无人管理被黑客利用。
<br>
如下一些常见的系统功能账号:
```
#file: /etc/passwd
bin:x:1:1:bin:/bin:/bin/bash
daemon:x:2:2:Daemon:/sbin:/bin/bash
games:x:12:100:Games account:/var/games:/bin/bash
ldap:x:76:70:User for OpenLDAP:/var/lib/ldap:/bin/bash
lp:x:4:7:Printing daemon:/var/spool/lpd:/bin/bash
man:x:13:62:Manual pages viewer:/var/cache/man:/bin/bash
news:x:9:13:News system:/etc/news:/bin/bash
nobody:x:65534:65533:nobody:/var/lib/nobody:/bin/bash
uucp:x:10:14:Unix-to-Unix CoPy system:/etc/uucp:/bin/bash
```
禁用账号方法:
```
root@sles:~ # usermod -L <username>    //锁定账号
root@sles:~ # which nologin
/sbin/nologin
root@sles:~ # usermod -s /sbin/nologin <username>    //修改用户shell为/sbin/nologin或者/bin/false，禁止用户登录系统
root@sles:~ # 
```
通常系统功能账号的UID大于0小于500，可以使用下面脚本禁用UID大于0小于500的账号:
```
for TMPUSER in $(awk -F: '(($3 > 0) && ($3 < 500)){print $1}' /etc/passwd);do
    usermod -L $TMPUSER 2>/dev/null
    usermod -s /sbin/nologin $TMPUSER 2>/dev/null
done
```
例外情况，nobody用户的UID>500，但一般用于NFS共享时使用，也要将其禁用掉:
```
root@sles:~ # usermod -L nobody
root@sles:~ # usermod -s nobody
```
