## 审计
### 1. audit
#### 1.1 简述
Linux审计功能根据设置的审计规则监控系统事件并记录日志。<br>
* 系统服务<br>
`auditd`
* 配置文件<br>
```
/etc/audit/auditd.conf
/etc/audit/audit.rules
```
* 常用命令<br>
```
auditctl
aureport
ausearch
```

#### 1.2 审计服务配置&开启
* 配置文件<br>
`/etc/audit/auditd.conf`

常用配置参数:<br>
```
log_file     : 指定审计日志文件位置
log_format   : RAW(default) | NOLOG(auditd进程不写日志，通过dispatcher去分发审计事件)
max_log_file_action  : 通常设置为rotate，保留一定量的审计日志，避免占用太多磁盘空间
max_log_file : 设置单个审计日志文件的大小
max_log_file : 设置保留多少个审计日志文件（max_log_file_action为rotate时生效）
flush & freq : 控制审计日志写入磁盘的方式、频率
dispatcher   : 审计事件分发器（可以实现向syslog发送审计日志）
```

开启审计服务:<br>
`service auditd start|restart`

#### 1.3 审计规则配置
可通过直接编辑审计规则配置文件或`auditctl`命令修改审计规则。<br>
* 配置文件<br>
`/etc/audit/audit.rules`
审计规则分为3类:<br>
* 1) 审计服务控制<br>
该类规则通常在文件起始位置，如:<br>
```
# First rule - delete all
-D

# Increase the buffers to survive stress events.
# Make this bigger for busy systems
-b 320
```

另外，在文件的最后一行通常配置`-e`规则:<br>
```
-e [0|1|2]
0: 禁用审计功能
1: 启用审计功能
2: 启用审计功能，并且审计服务/规则不可更改
```

* 2) 监控系统文件<br>
监视对指定文件或目录的访问。<br>
语法:<br>
```
-w path-to-file -p permissions -k keyname

permissions取值范围:
r - read of the file
w - write to the file
x - execute the file
a - change in the file's attribute
```

示例:<br>
```
#监视对sudo命令的执行
-w /usr/bin/sudo -p x -k sudo_run
```

* 3) 监控系统调用<br>
语法:<br>
```
-a action,list -S syscall -F field=value -k keyname

action可选值:
always - always create an event(常用)
never  - never create an event

list可选值:
task     -  during the fork or clone syscalls
exit     -  all syscall and file system audit requests are evaluated(常用)
user     -  events that originate in user space
exclude  -  exclude certain events from being emitted

-F可设置过滤规则
```

示例:<br>
```
#监视对内核模块的加载和卸载
-a always,exit -S init_module -S delete_module -F arch=b64 -k kernel_modules
```

#### 1.4 查看审计日志
审计日志通常存在于/var/log/audit/目录下，可直接查看该目录下的文件内容，也可以使用aureport或ausearch查看感兴趣的内容。<br>
如:<br>
```
aureport -k sudo_run
ausearch -k sudo_run
```

审计日志一般格式:<br>
```
type=SYSCALL msg=audit(1499741583.751:46199): arch=c000003e syscall=59 __success=yes__ __exit=0__ 
a0=1da3f70 a1=1fa3970 a2=1fa2ef0 a3=fc2c9fc5 items=2 ppid=21638 pid=21701 
__auid=0__ __uid=1000__ __gid=1000__ euid=0 suid=0 fsuid=0 egid=1000 sgid=1000 fsgid=1000 tty=pts0 ses=1 
__comm="sudo"__ __exe="/usr/bin/sudo"__ subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 __key="sudo_run"__
type=EXECVE msg=audit(1499741583.751:46199): argc=2 a0="sudo" a1="-l"
```
根据success、exit判断执行结果<br>
根据uid、gid等判断执行的用户<br>
根据comm、exe判断执行的文件路径<br>
根据key可以过滤指定类的审计日志<br>
