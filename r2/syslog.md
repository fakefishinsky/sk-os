## 简述
Linux系统syslog服务可以对用户行为、系统事件进行日志记录，对系统安全审计/监控、入侵检测等有重要作用。<br>

* 系统服务<br>
`syslog`<br>
`rsyslog`<br>
* 常用软件<br>
`rsyslog`<br>
`syslog-ng`<br>

## 日志分类&级别
* 常用日志分类<br>
**auth** : security/authorization messages<br>
**authpriv** : security/authorization messages <br>
**cron** : clock daemon (cron and at)<br>
**daemon** : system daemons without separate facility value<br>
**kern** : kernel messages (these can't be generage from user processes)<br>
**mail** : mail subsystem<br>
**news** : news subsystem<br>
**syslog** : messages generated internally by syslogd<br>
**user** : generic user-level messages<br>
**local0 ~ local7** : reserved for local use<br>

* 常用日志级别<br>
日志详细程度从上到下依次降低。<br>
**emerg** : system is unusable<br>
**alert** : action must be taken immediately<br>
**crit** : critical conditions<br>
**err** : error conditions<br>
**warn** : warning conditions<br>
**notice** : normal, but significant, condition<br>
**info** : informational message(常用)<br>
**debug** : debug-level message<br>

## rsyslog
* 配置文件<br>
`/etc/rsyslog.conf`<br>
`/etc/rsyslog.d/*.conf`<br>
<br>
* 常用配置<br>
`$umask 0077 #生成日志文件权限600`<br>
`*.*;mail.none;news.none　　-/var/log/messages # 除mail和news之外的所有日志`<br>
`auth.*;authpriv.*　　　　　-/var/log/secure # 认证/鉴权相关`<br>
`cron.*　　　　　　　　　　　-/var/log/cron # 定时任务相关日志`<br>
更多详细配置参考`man rsyslog.conf`。<br>

* 日志服务器<br>
若作为日志服务器来接收其他远程主机发送的日志，需修改`/etc/rsyslog.conf`配置:<br>
`#udp和tcp选择配置一个`<br>
`# tcp协议`<br>
`$ModLoad imtcp`<br>
`$InputTCPServerRun 514`<br>
`# udp协议`<br>
`$ModLoad imudp`<br>
`$UDPServerRun 514`<br>

* 发送日志到远程日志服务器<br>
若要将本地日志发送到远程日志服务器，需修改`/etc/rsyslog.conf`配置:<br>
`*.* @@server_ip:port # tcp协议`<br>
`*.* @server_ip:port # udp 协议`<br>
并编辑`/etc/sysconfig/syslog`文件，修改:<br>
`SYSLOG_REQUIRES_NETWORK=yes`

* 启动/重启服务<br>
`service rsyslog start|restart`<br>
## syslog-ng
* 配置文件<br>
`/etc/syslog-ng/syslog-ng.conf`<br>

* 常用配置<br>
`# 认证/鉴权相关日志`<br>
`filter f_secure { facility(auth, authpriv); };`<br>
`destination d_secure { file("/var/log/secure"); };`<br>
`log { source(src); filter(f_secure); destination(d_secure); };`<br>
<br>
`# 记录除mail/news/iptables之外的日志`
`filter f_messages   { not facility(news, mail) and not filter(f_iptables); };`<br>
`destination messages { file("/var/log/messages"); };`<br>
`log { source(src); filter(f_messages); destination(messages); };`<br>
更多详细配置参考`man syslog-ng.conf`。<br>

* 日志服务器<br>
若作为日志服务器来接收其他远程主机发送的日志，需配置:<br>
`source src {`<br>
` #udp和tcp选择配置一个`<br>
` udp("10.10.10.10" port(514)); # udp协议`<br>
` tcp("10.10.10.10" port(514)); # tcp协议`<br>
`};`<br>

* 发送日志到远程日志服务器<br>
若要将本地日志发送到远程日志服务器，需配置:<br>
`#udp和tcp选择配置一个`<br>
`destination udplogserver { udp("10.10.10.10" port(514)); };`<br>
`log { source(src); destination(udplogserver); };`<br><br>
`destination tcplogserver { tcp("10.10.10.10" port(514)); };`<br>
`log { source(src); destination(tcplogserver); };`<br>

* 启动/重启服务<br>
`service syslog start|restart`<br>
## logrotate
使用logrotate可以对系统日志进行归档清理，可以保留一定量的日志文件，避免日志文件不断增长过多占用磁盘空间。

* 定时任务<br>
脚本`/etc/cron.daily/logrotate`负责清理，每天执行一次。<br>

* logrotate<br>
配置文件在`/etc/logrotate.conf`和`/etc/logrotate.d/`下。关于syslog日志清理的配置文件一般为`/etc/logrotate.d/syslog`。<br>
maxage : 日志文件最大保留期限<br>
rotate : 日志文件最大保留个数<br>
create 0600 root root : 设置新生成日志文件的权限<br>