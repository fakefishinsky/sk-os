## Logging
Linux系统syslog服务可以对用户行为、系统事件进行日志记录，对系统安全审计/监控、入侵检测等有重要作用。
![logging-logo](images/logging-logo.jpg)
### 日志分类&级别
* 常用日志分类
|类别|描述|
|----|----|
|auth|security/authorization messages|
|authpriv|security/authorization messages|
|cron|clock daemon (cron and at)|
|daemon|system daemons without separate facility value|
|kern|kernel messages (these can't be generage from user processes)|
|mail|mail subsystem|
|news|news subsystem|
|syslog|messages generated internally by syslogd|
|user|generic user-level messages|
|localN(0~7)|reserved for local use|
* 常用日志级别
日志详细程度从上到下依次增加。
|级别|描述|
|----|----|
|emerg|system is unusable|
|alert|action must be taken immediately|
|crit|critical conditions|
|err|error conditions|
|warn|warning conditions|
|notice|normal, but significant, condition|
|info|informational message(常用)|
|debug|debug-level message|

### rsyslog
* 配置文件
`/etc/rsyslog.conf`
`/etc/rsyslog.d/*.conf`
* 常用配置
```
$umask 0077 #新生成文件(包括日志文件)权限设置为600
auth.*;authpriv.*　　　　　 -/var/log/secure    #认证/鉴权相关
cron.*　　　　　　　　　　　-/var/log/cron    #定时任务相关日志
*.*;auth.none;authpriv.none;cron.none;mail.none;news.none　　-/var/log/messages    #除auth、cron、mail和news之外的所有日志
```
更多详细配置请参考`man rsyslog.conf`。
* 日志服务器
若作为日志服务器来接收其他远程主机发送的日志，需配置:

```
#udp和tcp选择配置一个
#tcp协议
$ModLoad imtcp
$InputTCPServerRun 514    #开启的端口，可自定义 

#udp协议
$ModLoad imudp
$UDPServerRun 514
```

* 发送本地日志到远程日志服务器
若要将本地日志发送到远程日志服务器，需配置:

```
#udp和tcp选择配置一个
#tcp协议
*.* @@server_ip:port

#udp 协议
*.* @server_ip:port
```
并编辑`/etc/sysconfig/syslog`文件，修改:`SYSLOG_REQUIRES_NETWORK=yes`
* 启动/重启服务
`root:~ # service rsyslog start|restart`

### syslog-ng
* 配置文件
`/etc/syslog-ng/syslog-ng.conf`
* 常用配置

```
#Global options. perm设置新生成日志文件权限为600
options { long_hostnames(off); sync(0); perm(0600); stats(3600); };

#认证/鉴权相关日志
filter f_secure { facility(auth, authpriv); };
destination d_secure { file("/var/log/secure"); };
log { source(src); filter(f_secure); destination(d_secure); };

#定时任务相关日志
filter f_cron { facility(cron); };
destination d_cron { file("/var/log/cron"); };
log { source(src); filter(f_cron); destination(d_cron); };

#记录除auth/cron/mail/news/iptables之外的日志
filter f_messages   { not facility(auth, authpriv, cron, news, mail) and not filter(f_iptables); };
destination messages { file("/var/log/messages"); };
log { source(src); filter(f_messages); destination(messages); };
```
更多详细配置参考`man syslog-ng.conf`。
* 日志服务器
若作为日志服务器来接收其他远程主机发送的日志，需配置:

```
source src {
	#......
	#udp和tcp选择配置一个
	udp("10.10.10.10" port(514));   #udp协议
	tcp("10.10.10.10" port(514));   #tcp协议
};
```
* 发送本地日志到远程日志服务器
若要将本地日志发送到远程日志服务器，需配置:

```
#udp和tcp选择配置一个
#udp协议
destination udplogserver { udp("10.10.10.10" port(514)); };
log { source(src); destination(udplogserver); };

#tcp协议
destination tcplogserver { tcp("10.10.10.10" port(514)); };
log { source(src); destination(tcplogserver); };
```
* 启动/重启服务
`root:~ # service syslog start|restart`

### logrotate
使用logrotate可以对系统日志进行归档清理，可以保留一定量的日志文件，避免日志文件不断增长过多占用磁盘空间。
* 定时任务
脚本`/etc/cron.daily/logrotate`负责清理，每天执行一次。
* logrotate
配置文件在`/etc/logrotate.conf`和`/etc/logrotate.d/`下。关于syslog日志清理的配置文件一般为`/etc/logrotate.d/syslog`。
```
node-2:~ # cat /etc/logrotate.d/syslog
/var/log/messages /var/log/cron /var/log/secure    #指定要归档的日志文件路径
{
    compress    #压缩归档
    dateext    #压缩文件以日期作为后缀名
    maxage 365    #日志文件最大保留期限
    rotate 99    #日志文件最大保留个数
    missingok
    notifempty
    size +4096k    #日志文件大小达到4096K才满足归档条件
    create 0600 root root    #设置新生成日志文件的权限
    sharedscripts
    postrotate
        /usr/bin/systemctl reload syslog.service > /dev/null
    endscript
}
```