### Linux PROC 文件系统详细介绍
#### 什么是 proc 文件系统？
proc 文件系统是一个伪文件系统，它只存在内存当中，而不占用外存空间。它以文件系统的方式为访问系统内核数据的操作提供接口。
用户和应用程序可以通过 proc 得到系统的信息，并可以改变内核的某些参数。由于系统的信息，如进程，是动态改变的，所以用户
或应用程序读取 proc 文件时，proc 文件系统是动态从系统内核读出所需信息并提交的。

#### 常见目录结构
|名称|信息|
|----|-----|
|apm|高级电源管理信息|
|cmdline|内核命令行|
|cpuinfo|关于 Cpu 信息|
|devices|可以用到的设备（块设备/字符设备）|
|dma|使用的 DMA 通道|
|filesystems|支持的文件系统|
|interrupts|中断的使用|
|ioports|I/O 端口的使用|
|kcore|内核核心印象|
|kmsg|内核消息|
|ksyms|内核符号表|
|loadavg|负载均衡|
|locks|内核锁|
|meminfo|内存信息|
|misc|杂项|
|modules|加载模块列表|
|mounts|加载的文件系统|
|partitions|系统识别的分区表|
|rtc|实时时钟|
|slabinfo|Slab 池信息|
|stat|全面统计状态表|
|swaps|对换空间的利用情况|
|version|内核版本|
|uptime|系统正常运行时间|

并不是所有这些目录在你的系统中都有，这取决于你的内核配置和装载的模块。另外，在/proc 下还有三个很重要的目录：net，scsi 和 sys。 
Sys 目录是可写的，可以通过它来访问或修改内核的参数（见下一部分），而 net 和 scsi 则依赖于内核配置。例如，如果系统不支持 scsi，
则 scsi 目录不存在。

#### 进程目录的结构
除了以上介绍的这些，还有的是一些以数字命名的目录，它们是进程目录。系统中当前运行的每一个进程都有对应的一个目录在/proc 下，
以进程的 PID 号为目录名，它们是读取进程信息的接口。而 self 目录则是读取进程本身的信息接口，是一个 link。Proc 文件系
统的名字就是由之而起。

|名称|信息|
|----|-----|
|cmdline|命令行参数|
|environ|环境变量值|
|fd|一个包含所有文件描述符的目录|
|mem|进程的内存被利用情况|
|stat|进程状态|
|status|进程当前状态，以可读的方式显示出来|
|cwd|当前工作目录的链接|
|exe|指向该进程的执行命令文件|
|maps|内存映象|
|statm|进程内存状态信息|
|root|链接此进程的 root 目录|

#### 修改内核参数
在/proc 文件系统中有一个有趣的目录：/proc/sys。
它不仅提供了内核信息，而且可以通过它修改内核参数，来优化你的系统。但是你必须很小
心，因为可能会造成系统崩溃。最好是先找一台无关紧要的机子，调试成功后再应用到你的
系统上。
要改变内核的参数，只要用 vi 编辑或 echo 参数重定向到文件中即可。下面有一个例
子：
# cat /proc/sys/fs/file-max
4096
# echo 8192 > /proc/sys/fs/file-max
# cat /proc/sys/fs/file-max
8192 
如果你优化了参数，则可以把它们写成添加到文件 rc.local 中，使它在系统启动时自
动完成修改。

#### 网络参数
在/proc/sys/net/ipv4/目录下，包含的是和 tcp/ip 协议相关的各种参数，下面我们
就对这些网络参数加以详细的说明。
|---|---|---|---|
|参数名|意义|类型|取值|
|ip_forward|在网络本地接口之间转发数据报。<br>对该参数的修改将导致其它所有相关配置参数恢复其默认值<br>(对于主机参阅 RFC1122，对于路由器参见 RFC1812)|布尔|0 - 关闭(默认值)<br>not 0 - 打开 ip 转发|
|ip_default_ttl|表示 IP 数据报的 Time To Live 值|整型|默认值为 64|
|ip_no_pmtu_disc|关闭路径 MTU 探测|布尔|默认值为 FALSE|
|ipfrag_high_thresh|用来组装分段的 IP 包的最大内存量。<br>当 ipfrag_high_thresh 数量的内存被分配用来组装 IP 包，则 IP 分片处理器将丢弃数据报<br>直到 ipfrag_low_thresh 数量的内存被用来组装 IP 包。|整型|-|
|ipfrag_low_thresh|参见 ipfrag_high_thresh|整型|-|
|ipfrag_time|保存一个 IP 分片在内存中的时间|整型|-|
|inet_peer_threshold|INET 对端存储器某个合适值，当超过该阀值条目将被丢弃。<br>该阀值同样决定生存时间以及废物收集通过的时间间隔。<br>条目越多﹐存活期越低﹐GC 间隔越短|整型|-|
|inet_peer_minttl|条目的最低存活期。<br>在重组端必须要有足够的碎片(fragment)存活期。<br>这个最低存活期必须保证缓冲池容积是否少于 inet_peer_threshold。<br>该值以 jiffies 为单位测量。|整型|-|
|inet_peer_maxttl|条目的最大存活期。<br>在此期限到达之后﹐如果缓冲池没有耗尽压力的话﹐不使用的条目将会超时。<br>该值以 jiffies 为单位测量。|整型|-|
|inet_peer_gc_mintime|废物收集(GC)通过的最短间隔。<br>这个间隔会影响到缓冲池中内存的高压力。<br>该值以 jiffies为单位测量。|整型|-|
|inet_peer_gc_maxtime|废物收集(GC)通过的最大间隔。<br>这个间隔会影响到缓冲池中内存的低压力。<br>该值以 jiffies为单位测量。|整型|-|
|tcp_syn_retries|对于一个新建连接，内核要发送多少个 SYN 连接请求才决定放弃。|整型|不应该大于 255<br>默认值是 5，对应于 180 秒左右|
|tcp_synack_retries|对于远端的连接请求 SYN，内核会发送 SYN ＋ ACK 数据报，<br>以确认收到上一个 SYN 连接请求包。这是所谓的三次握手机制的第二个步骤。<br>该参数决定内核在放弃连接之前所送出的 SYN+ACK 数目。|整型|-|
|tcp_keepalive_time|当 keepalive 打开的情况下，TCP 发送 keepalive 消息的频率|整型|默认值是 2 个小时|
|tcp_keepalive_probes|TCP 发送 keepalive 探测以确定该连接已经断开的次数|整型|默认值是 9|
|tcp_keepalive_interval|探测消息发送的频率。<br>乘以 tcp_keepalive_probes 就得到对于从开始探测以来没有响应的连接杀除的时间。|整型|默认值为 75 秒|
|tcp_retries1|当出现可疑情况而必须向网络层报告这个可疑状况之前﹐需要进行多少次重试。|整型|默认值是 3|
|tcp_retries2|在丢弃激活的 TCP 连接之前﹐需要进行多少次重试。|整型|默认值为 15|
|tcp_orphan_retries|在近端丢弃 TCP 连接之前﹐要进行多少次重试。|整型|默认值是 7|
|tcp_fin_timeout|对于本端断开的 socket 连接，TCP 保持在 FIN-WAIT-2 状态的时间。<br>对方可能会断开连接或一直不结束连接或不可预料的进程死亡。|整型|默认值为 60 秒|
|tcp_max_tw_buckets|系统同时处理的最大 timewait sockets 数目。<br>如果超过此数的话﹐time-wait socket 会被立即砍除并且显示警告信息。<br>设定这个限制﹐可以抵御简单的 DoS 攻击﹐千万不要人为的降低这个限制。|整型|-|
|tcp_tw_recycle|打开快速 TIME-WAIT sockets 回收。|布尔|默认值是 1|
|tcp_max_orphans|系统所能处理不属于任何进程的 TCP sockets 最大数量。<br>设定这个限制﹐可以抵御简单的 DoS 攻击|整型|-|
|tcp_abort_on_overflow|当守护进程太忙而不能接受新的连接，就象对方发送 reset 消息|布尔|默认值是 false|
|tcp_syncookies|当出现 syn 等候队列出现溢出时象对方发送 syncookies。<br>目的是为了防止 syn flood 攻击，但在高负载服务器上会存在误报。<br>该参数可能对某些服务导致严重的性能影响。|布尔|默认值是 false|
|tcp_stdurg|使用 TCP urg pointer 字段中的主机请求解释功能。|整型|默认值为为﹕FALSE|
|tcp_max_syn_backlog|对于那些依然还未获得客户端确认的连接请求﹐需要保存在队列中最大数目。|整型|默认值是 1024|
|tcp_window_scaling|正常来说，TCP/IP 可以接受最大到 65535 字节的 windows。<br>对于宽带网络，该值可能是不够的，通过调整该参数有助于提高宽带服务器性能。|布尔|-|
|tcp_timestamps|可以防范伪造的 sequence 号码|布尔|-|
|tcp_sack|使用 Selective ACK﹐它可以用来查找特定的遗失的数据报，因此有助于快速恢复状态。|布尔|-|
|tcp_fack|打开 FACK 拥塞避免和快速重传功能。|布尔|-|
|tcp_dsack|允许 TCP 发送"两个完全相同"的 SACK。|布尔|-|
|tcp_ecn|打开 TCP 的直接拥塞通告功能。|布尔|-|
|tcp_reordering|TCP 流中重排序的数据报最大数量|整型|默认值是 3|
|tcp_retrans_collapse|对于某些有 bug 的打印机提供针对其 bug 的兼容性。|布尔|-|
|tcp_wmem|min：为 TCP socket 预留用于发送缓冲的内存最小值。<br>default：为 TCP socket 预留用于发送缓冲的内存数量<br>max: 用于 TCP socket 发送缓冲的内存最大值。|三个整数的向量:min default max|默认值为 4K 16K 128K|
|tcp_rmem|min：为 TCP socket 预留用于接收缓冲的内存最小值<br>default：为 TCP socket 预留用于接收缓冲的内存数量<br>max：用于 TCP socket 接收缓冲的内存最大值|三个整数的向量:min default max|-|
|tcp_mem|low：当 TCP 使用了低于该值的内存页面数时，TCP 不会考虑释放内存。<br>pressure：当 TCP 使用了超过该值的内存页面数量时，TCP 试图稳定其内存使用，进入 pressure模式，<br>当内存消耗低于 low 值时则退出 pressure 状态。<br>high：允许所有 tcp sockets 用于排队缓冲数据报的页面量。|三个整数的向量:low pressure high|-|
|ip_local_port_range|定于 TCP 和 UDP 使用的本地端口范围，第一个数是开始，第二个数是最后端口号|两个整数|-|
|icmp_echo_ignore_all|忽略所有的icmp echo消息|布尔|-|
|icmp_echo_ignore_broadcasts|忽略icmp echo广播消息|布尔|-|
|icmp_ignore_bogus_error_responses|记录icmp错误消息日志|布尔|默认值为 False|
|log_martians|记录带有不允许的地址的数据报到内核日志中|布尔|-|
|accept_redirects|接收 ICMP 重定向消息。|布尔|-|
|forwarding|在该接口打开转发功能|布尔|-|
|mc_forwarding|是否进行多播路由。|布尔|-|
|proxy_arp|打开 proxy arp 功能|布尔|-|
|shared_media|发送(路由器)或接收(主机) RFC1620 共享媒体重定向<br>覆盖secure_redirects的值|布尔|默认为True|
|secure_redirects|仅仅接收发给默认网关列表中网关的 ICMP 重定向消息|布尔|默认值是 TRUE|
|send_redirects|发送重定向消息|布尔|默认值是 TRUE|
|accept_source_route|接收带有 SRR 选项的数据报|布尔|-|
|rp_filter|通过反向路径回溯进行源地址验证|布尔|0: 关闭该功能(默认值)<br>1: 开启该功能|
