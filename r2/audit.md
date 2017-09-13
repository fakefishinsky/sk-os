## Audit
Linux审计功能根据设置的审计规则监控系统事件并记录日志，对入侵检测和事后审计有重要意义。
![audit-logo](images/audit-logo.jpg)
### 审计服务配置&开启
* 配置文件
`/etc/audit/auditd.conf`
* 常用配置参数
|参数|意义|
|----|----|
|log_file|指定审计日志文件位置|
|log_format|RAW(default)<br>NOLOG(auditd进程不写日志，通过dispatcher去分发审计事件)|
|max_log_file_action|通常设置为rotate，保留一定量的审计日志，避免占用太多磁盘空间|
|max_log_file|设置单个审计日志文件的大小|
|num_logs|设置保留多少个审计日志文件（max_log_file_action为rotate时生效）|
|flush & freq|控制审计日志写入磁盘的方式、频率|
|dispatcher|审计事件分发器（可以实现向syslog发送审计日志）|
* 开启审计服务
`service auditd start|restart`

### 审计规则配置
可直接编辑审计规则配置文件(`/etc/audit/audit.rules`)或通过`auditctl`命令修改审计规则。
审计规则可分为3类:
* 1) 审计服务控制
该类规则通常在文件起始位置，如:
```
# First rule - delete all
-D
# Increase the buffers to survive stress events.
# Make this bigger for busy systems
-b 320
```
另外，在文件的最后一行通常配置`-e`规则:
```
# -e 0: 禁用审计功能
# -e 1: 启用审计功能
# -e 2: 启用审计功能，并且审计服务/规则不可更改
-e [0|1|2]
```
* 2) 监控系统文件
监视对指定文件或目录的访问。
语法:`-w path-to-file -p permissions -k keyname`<br>
permissions取值范围:
r - read of the file
w - write to the file
x - execute the file
a - change in the file's attribute<br>
示例:
```
#监视对sudo命令的执行
-w /usr/bin/sudo -p x -k sudo_run
```
* 3) 监控系统调用
语法:`-a action,list -S syscall -F field=value -k keyname`<br>
action可选值:
always - always create an event(常用)
never  - never create an event<br>
list可选值:
task     -  during the fork or clone syscalls
exit     -  all syscall and file system audit requests are evaluated(常用)
user     -  events that originate in user space
exclude  -  exclude certain events from being emitted<br>
-F可设置过滤规则<br>
示例:
```
#监视对内核模块的加载和卸载
-a always,exit -S init_module -S delete_module -F arch=b64 -k kernel_modules
```

### 查看审计日志
审计日志通常存在于`/var/log/audit/`目录下，可直接查看该目录下的文件内容，也可以使用aureport或ausearch查看感兴趣的内容。
如:
```
root@suse:~ # aureport -k sudo_run    //key=sudo_run
root@suse:~ # ausearch -k sudo_run
```
* 审计日志一般格式
```
type=SYSCALL msg=audit(1499741583.751:46199): arch=c000003e syscall=59 success=yes exit=0
a0=1da3f70 a1=1fa3970 a2=1fa2ef0 a3=fc2c9fc5 items=2 ppid=21638 pid=21701
auid=0 uid=1000 gid=1000 euid=0 suid=0 fsuid=0 egid=1000 sgid=1000 fsgid=1000 tty=pts0 ses=1
comm="sudo" exe="/usr/bin/sudo" subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 key="sudo_run"
type=EXECVE msg=audit(1499741583.751:46199): argc=2 a0="sudo" a1="-l"
```
根据success、exit判断执行结果
根据uid、gid等判断执行的用户
根据comm、exe判断执行的文件路径
根据key可以过滤指定类的审计日志

### 审计日志上传日志服务器
Linux审计功能默认不支持向远程日志服务器发送日志，需借助dispatcher进行审计事件分发。
在`/etc/audit/auditd.conf`文件中有如下配置参数:
```
dispatcher = /sbin/audispd
```
audispd负责审计事件分发，在/etc/audisp/目录下有相关配置文件。
要实现审计日志上传日志服务器，需将本地审计事件分发到本地syslog，然后借助syslog的功能将本地日志上传到日志服务器。
编辑`/etc/audisp/plugins.d/syslog.conf`文件，修改:
```
active = yes
#......
args = LOG_INFO LOG_LOCAL0
```
设置之后需重启审计服务:
```
root@suse:~ # service auditd restart
```
之后就可以在本地syslog服务的配置文件中对local0.info类的日志进行处理。
如，在syslog-ng中（`/etc/syslog-ng/syslog-ng.conf`）进行如下配置:
```
filter f_auditd { facility(local0) and level(info); };
destination logserver { udp("X.X.X.X", port(514)); };
log { source(src); filter(f_auditd); destination(logserver); };
```
然后重启syslog服务:
```
root@suse:~ # service syslog restart
```