#### 19)CAP_NET_RAW
Capability: <br>
Use RAW and PACKET sockets.<br>
允许使用原始(raw)套接字

Test:<br>
普通用户使用tcpdump进行抓包

```
#赋能前，无法抓包
[fjl@Redhat ~]$ tcpdump -c 4
tcpdump: no suitable device found
[fjl@Redhat ~]$ exit
logout

#赋能后，普通用户可以tcpdump抓包
[root@Redhat ~]# setcap CAP_NET_RAW=eip /usr/sbin/tcpdump
[root@Redhat ~]# getcap /usr/sbin/tcpdump
/usr/sbin/tcpdump = cap_net_raw+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ tcpdump -c 4
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on eth0, link-type EN10MB (Ethernet), capture size 65535 bytes
22:02:24.437113 IP 192.168.1.20.ssh > 192.168.1.10.48793: Flags [P.], seq 3268036938:3268037134, ack 375322178, win 184, options [nop,nop,TS val 843344159 ecr 84518432], length 196
22:02:24.437231 IP 192.168.1.10.48793 > 192.168.1.20.ssh: Flags [.], ack 196, win 129, options [nop,nop,TS val 84518434 ecr 843344143], length 0
22:02:24.437380 IP 192.168.1.10.ssh > 192.168.1.30.65254: Flags [P.], seq 868215282:868215478, ack 3481478883, win 128, length 196
22:02:24.437699 IP 192.168.1.20.35422 > lg-ns3.huawei.com.domain: 49796+ PTR? 116.114.71.10.in-addr.arpa. (44)
4 packets captured		//抓包成功
19 packets received by filter
0 packets dropped by kernel
```
需要CAP_NET_RAW能力的还有ping等。

#### 20)CAP_SETGID
Capability: <br>
Make  arbitrary  manipulations of process GIDs and supplementary GID list; forge GID when passing  socket  credentials  via  UNIX domain sockets.
<br>设定程序允许普通用户使用setgid函数，与文件的SGID权限位无关

Test:

```
#setgidtest程序代码
[root@Redhat ~]# cat setgidtest.c 
#include <unistd.h>
void main()
{
        gid_t gid=0;
        setgid(gid);	//调用setgid函数
        system("/bin/cat /etc/shadow");	//读取/etc/shadow文件内容
}

#赋能前，setgid(0)调用失败
[fjl@Redhat ~]$ /tmp/setgidtest 
/bin/cat: /etc/shadow: Permission denied		//读取失败

#赋能后，setgid(0)调用成功
[root@Redhat ~]# setcap CAP_SETGID=eip /tmp/setgidtest
[root@Redhat ~]# getcap /tmp/setgidtest
/tmp/setgidtest = cap_setgid+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ ll /etc/shadow
-rw-r-----. 1 root root 1318 Nov 15 01:07 /etc/shadow
[fjl@Redhat ~]$ /tmp/setgidtest		//读取成功
root:$6$xxxx.:16705:7:99999:7:::
fjl:$6$xxxx:16752:7:90:7:::
```

#### 21)CAP_SETFCAP
Capability: <br>
Set file capabilities.<br>
允许在指定的程序上授权能力给其它程序

Test:<br>
普通用户给任意程序赋能

```
[fjl@Redhat ~]$ setcap CAP_CHOWN=eip /bin/chown
unable to set CAP_SETFCAP effective capability: Operation not permitted	  //赋能失败
[fjl@Redhat ~]$ exit
logout
[root@Redhat ~]# setcap CAP_SETFCAP=eip /usr/sbin/setcap
[root@Redhat ~]# getcap /usr/sbin/setcap
/usr/sbin/setcap = cap_setfcap+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ setcap CAP_CHOWN=eip /bin/chown
[fjl@Redhat ~]$ getcap /bin/chown
/bin/chown = cap_chown+eip		//普通用户成功给/bin/chown赋能CAP_CHOWN
```

#### 22)CAP_SETPCAP
Capability: <br>
If  file  capabilities  are  not  supported: grant or remove any capability in the caller's permitted capability set to  or  from any  other process.  (This property of CAP_SETPCAP is not available when the kernel is configured to support file capabilities, since CAP_SETPCAP has entirely different semantics for such kernels.)
<br>If file capabilities are supported: add any capability from  the calling thread's bounding set to its inheritable set; drop capabilities from the bounding set (via  prctl(2)  PR_CAPBSET_DROP);
make changes to the securebits flags.<br>
允许向其它进程转移能力以及删除其它进程的任意能力<br>
只有init进程可以设定其它进程的能力，而其它程序无权对进程授权，root用户也不能对其它进程的能力进行修改，只能对当前进程通过cap_set_proc等函数进行修改，而子进程也会继承这种能力，所以即使使用了CAP_SETPCAP能力，也不会起到真正的作用。

#### 23)CAP_SETUID
Capability: 
* perform operations on trusted and security Extended Attributes (see attr(5));
* use lookup_dcookie(2);
* use ioprio_set(2) to assign IOPRIO_CLASS_RT and (before  Linux 2.6.25) IOPRIO_CLASS_IDLE I/O scheduling classes;
* forge UID when passing socket credentials;
* exceed  /proc/sys/fs/file-max,  the  system-wide  limit on the number of open files, in system calls that open  files  (e.g., accept(2), execve(2), open(2), pipe(2));
* employ CLONE_NEWNS flag with clone(2) and unshare(2);
* call setns(2);
* perform KEYCTL_CHOWN and KEYCTL_SETPERM keyctl(2) operations;
* perform madvise(2) MADV_HWPOISON operation.
<br>设定程序允许普通用户使用setuid函数，与文件的SUID权限位无关

Test:

```
#setuidtest程序代码
[root@Redhat ~]# cat setuidtest.c 
#include <unistd.h>
void main()
{
        uid_t uid=0;
        setuid(uid);		//调用setuid函数
        system("/usr/bin/whoami");	//执行whoami命令
}

#赋能前，setuid(0)执行失败，whoami返回普通用户fjl
[fjl@Redhat ~]$ /tmp/setuidtest
fjl

#赋能后，setuid(0)执行成功，whoami返回root
[root@Redhat ~]# setcap CAP_SETUID=eip /tmp/setuidtest
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ /tmp/setuidtest
root
```
#### 24)CAP_SYS_ADMIN
Capability: 
* Perform a range of system administration operations including:
quotactl(2),  mount(2),  umount(2),   swapon(2),   swapoff(2), sethostname(2), and setdomainname(2);
* perform  IPC_SET and IPC_RMID operations on arbitrary System V IPC objects;
* perform operations on trusted and security Extended Attributes (see attr(5));
* use lookup_dcookie(2);
* use  ioprio_set(2) to assign IOPRIO_CLASS_RT and (before Linux 2.6.25) IOPRIO_CLASS_IDLE I/O scheduling classes;
* forge UID when passing socket credentials;
* exceed /proc/sys/fs/file-max, the  system-wide  limit  on  the number  of  open files, in system calls that open files (e.g., accept(2), execve(2), open(2), pipe(2));
* employ CLONE_NEWNS flag with clone(2) and unshare(2);
* perform KEYCTL_CHOWN and KEYCTL_SETPERM keyctl(2)  operations.
<br>允许执行系统管理任务，如挂载/卸载文件系统，设置磁盘配额等

Test:<br>
使用普通用户修改主机名

```
[fjl@Redhat ~]$ hostname
Redhat

#赋能前，修改主机名失败
[fjl@Redhat ~]$ hostname test
hostname: you must be root to change the host name	 
[fjl@Redhat ~]$ exit
logout

#赋能后，普通用户成功修改主机名
[root@Redhat ~]# setcap CAP_SYS_ADMIN=eip /bin/hostname
[root@Redhat ~]# getcap /bin/hostname
/bin/hostname = cap_sys_admin+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ hostname
Redhat
[fjl@Redhat ~]$ hostname test
[fjl@Redhat ~]$ hostname
test		//成功修改主机名
```

#### 25)CAP_SYS_BOOT
Capability: <br>
Use reboot(2) and kexec_load(2).<br>
允许普通用户使用reboot函数和kexec_load函数

Test:

```
#reboottest程序代码（不能直接使用reboot或者shutdown命令，里面判断了执行者是否为root）
[root@Redhat ~]# cat reboottest.c
#include <unistd.h>
#include <sys/reboot.h>
void main()
{
        sync();
        reboot(RB_AUTOBOOT);		//调用reboot函数
}

#赋能前，reboot失败，系统并没有重启
[fjl@Redhat ~]$ /tmp/reboottest
[fjl@Redhat ~]$ echo $?
255		//reboot执行失败，系统没有重启

#赋能后，reboot成功，系统重启
[root@Redhat ~]# setcap CAP_SYS_BOOT=eip /tmp/reboottest
[root@Redhat ~]# getcap /tmp/reboottest
/tmp/reboottest = cap_sys_boot+eip
[fjl@Redhat ~]$ /tmp/reboottest
~ #		//已连接退出，系统已经开始重启
```

#### 26)CAP_SYS_CHROOT
Capability: <br>
Use chroot(2).<br>
允许使用chroot()系统调用

Test:<br>
普通用户fjl调用chroot

```
#赋能前，chroot失败
[fjl@Redhat ~]$ chroot / /bin/sh
chroot: cannot change root directory to /: Operation not permitted
[fjl@Redhat ~]$ exit
logout

#赋能后，普通用户使用chroot成功
[root@Redhat ~]# setcap CAP_SYS_CHROOT=eip /usr/sbin/chroot
[root@Redhat ~]# getcap /usr/sbin/chroot
/usr/sbin/chroot = cap_sys_chroot+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ chroot / /bin/sh
sh-4.1$ exit		//chroot成功
exit
```

#### 27)CAP_SYS_MODULE
Capability:<br>
Load   and   unload   kernel  modules  (see  init_module(2)  and delete_module(2)); in kernels before 2.6.25:  drop  capabilities from the system-wide capability bounding set.
<br>允许普通用户加载和卸载内核模块

Test:<br>
普通用户加载/卸载内核模块

```
#赋能前，普通用户fjl无法卸载模块
[root@Redhat ~]# lsmod | grep sg
sg                     29318  0
[fjl@Redhat ~]$ rmmod sg
ERROR: Removing 'sg': Operation not permitted	//卸载失败

#赋能后，普通用户fjl卸载模块成功
[root@Redhat ~]# setcap CAP_SYS_MODULE=eip /sbin/rmmod
[root@Redhat ~]# getcap /sbin/rmmod
/sbin/rmmod = cap_sys_module+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ rmmod sg		//卸载成功
[fjl@Redhat ~]$ lsmod | grep sg
[fjl@Redhat ~]$
```

#### 28)CAP_SYS_NICE
Capability:
* Raise  process nice value (nice(2), setpriority(2)) and change the nice value for arbitrary processes;
* set real-time scheduling policies for calling process, and set scheduling  policies  and  priorities  for arbitrary processes (sched_setscheduler(2), sched_setparam(2));
* set CPU  affinity  for  arbitrary  processes  (sched_setaffinity(2));
* set  I/O scheduling class and priority for arbitrary processes (ioprio_set(2));
* apply migrate_pages(2) to arbitrary processes and  allow  processes to be migrated to arbitrary nodes;
* apply move_pages(2) to arbitrary processes;
* use the MPOL_MF_MOVE_ALL flag with mbind(2) and move_pages(2).
<br>允许提升优先级，设置其它进程的优先级

Test:<br>
普通用户修改任意进程的优先级

```
#赋能前，重新设置进程优先级失败
[fjl@Redhat ~]$ ps aux | grep passwd | grep -v grep
root      3500  0.0  0.1 161876  2024 pts/0    S+   06:34   0:00 passwd lj
[fjl@Redhat ~]$ renice -n -5 3500
renice: 3500: setpriority: Operation not permitted    //设置进程优先级失败

#赋能后，普通用户fjl成功重新设置root用户进程优先级
[root@Redhat ~]# setcap CAP_SYS_NICE=eip /usr/bin/renice
[root@Redhat ~]# getcap /usr/bin/renice
/usr/bin/renice = cap_sys_nice+eip
[fjl@Redhat ~]$ ps aux | grep passwd | grep -v grep
root      3514  0.0  0.1 161876  2028 pts/0    S+   06:35   0:00 passwd lj
[fjl@Redhat ~]$ renice -n -5 3514
3514: old priority 0, new priority -5	//成功设置进程优先级
```

#### 29)CAP_SYS_PACCT
Capability:<br>
Use acct(2).<br>
允许配置进程记帐

Test:

```
#测试程序代码
[root@Redhat ~]# cat accttest.c 
#include <unistd.h>
#include <sys/acct.h>
#include <stdio.h>
void main()
{
        int ret = 0;
        ret = acct("/tmp/acct.log");		//开启进程记账
        if ( ret < 0 )
        {
                perror("acct error");
                return;
        }
        system("/usr/bin/whoami");
        ret = acct(NULL);			//关闭进程记账
        if ( ret < 0 )
        {
                perror("acct error");
                return;
        }
}

#赋能前，普通用户fjl无权使用acct
[fjl@Redhat ~]$ /tmp/accttest
acct error: Operation not permitted
[fjl@Redhat ~]$ exit
logout

#赋能后，acct调用成功
[root@Redhat ~]# setcap CAP_SYS_PACCT=eip /tmp/accttest
[root@Redhat ~]# getcap /tmp/accttest
/tmp/accttest = cap_sys_pacct+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ touch /tmp/acct.log
[fjl@Redhat ~]$ /tmp/accttest
fjl		//调用成功
[fjl@Redhat ~]$ lastcomm -f /tmp/acct.log		//程序记账成功
whoami      fjl      pts/0      0.00 secs Mon Nov 16 06:23	
```

#### 30)CAP_SYS_PTRACE
Capability:<br>
Trace  arbitrary  processes  using    ptrace(2);    apply get_robust_list(2) to arbitrary processes.<br>
允许跟踪任何进程

Test:<br>
使用普通用户跟踪任意进程

```
#赋能前，跟踪失败
[fjl@Redhat ~]$ ps aux | grep passwd | grep -v grep
root      3126  0.0  0.1 161876  2028 pts/0    S+   05:45   0:00 passwd lj
[fjl@Redhat ~]$ strace -p 3126
attach: ptrace(PTRACE_ATTACH, ...): Operation not permitted	

#赋能后，普通用户成功跟踪root用户进程
[root@Redhat ~]# setcap CAP_SYS_PTRACE=eip /usr/bin/strace
[root@Redhat ~]# getcap /usr/bin/strace
/usr/bin/strace = cap_sys_ptrace+eip
[fjl@Redhat ~]$ ps aux | grep passwd | grep -v grep
root      3167  0.0  0.1 161876  2024 pts/0    S+   05:47   0:00 passwd lj
[fjl@Redhat ~]$ strace -p 3167
Process 3167 attached - interrupt to quit		//跟踪root用户进程成功
read(0, "huawei\n", 511)                = 7
ioctl(0, SNDCTL_TMR_STOP or TCSETSW, {B38400 opost isig icanon echo ...}) = 0
write(2, "\n", 1)                       = 1
```

#### 31)CAP_SYS_RAWIO
Capability:<br>
Perform I/O port  operations  (iopl(2)  and  ioperm(2));  access /proc/kcore.<br>
允许用户打开端口，并读取修改端口数据，一般用ioperm/iopl函数

Test:

```
#iopltest程序代码
[root@Redhat ~]# vi iopltest.c
#include <unistd.h>
#include <sys/io.h>
#include <stdio.h>
void main()
{
        if( iopl(3) < 0 )	//调用iopl函数
        {
                perror("iopl set error");
                return;
        }
        if( iopl(0) < 0 )	//调用iopl函数
        {
                perror("iopl set error");
                return;
        }
}

#赋能前，普通用户调用iopl失败
[fjl@Redhat ~]$ /tmp/iopltest
iopl set error: Operation not permitted	

#赋能后，普通用户调用iopl成功
[root@Redhat ~]# setcap CAP_SYS_RAWIO=eip /tmp/iopltest
[root@Redhat ~]# getcap /tmp/iopltest
/tmp/iopltest = cap_sys_rawio+eip
[fjl@Redhat ~]$ /tmp/iopltest
[fjl@Redhat ~]$ echo $?
0	 //成功
```

#### 32)CAP_SYS_RESOURCE
Capability:
* Use reserved space on ext2 file systems;
* make ioctl(2) calls controlling ext3 journaling;
<br>忽略资源限制，limit限制，磁盘配额等等

Test:<br>
普通用户用setrlimit()来突破ulimit的限制

```
#测试程序setulimit代码
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
int main (int argc, char *argv[])
{
	int r = 0;
	struct rlimit rl;
	getrlimit (RLIMIT_STACK,&rl);
	printf("crrent hard limit is %ld\n",(u_long) rl.rlim_max);
	rl.rlim_max = rl.rlim_max+1;
	r = setrlimit (RLIMIT_STACK, &rl);	//调用setrlimit突破ulimit限制
	if (r)
	{
		perror("setrlimit");
		return -1;
	}
	printf("limit set to %ld \n", (u_long) rl.rlim_max+1);
	return 0;
}
   

#查看当前的限制,这里是10MB，赋能前，普通用户执行setrlimit()失败
[fjl@Redhat ~]$ ulimit -H -s 
10240
[fjl@Redhat ~]$ ./setulimit    
crrent hard limit is 10485760
setrlimit: Operation not permitted    //失败

#赋能后，普通用户执行setrlimit()成功，突破ulimit的限制
[root@Redhat ~]# setcap CAP_SYS_RESOURCE=eip /home/fjl/setulimit 
[fjl@Redhat ~]$ ./setulimit 
crrent hard limit is 10485760
limit set to 10485762   //成功突破ulimit的限制
```

#### 33)CAP_SYS_TIME
Capability:<br>
Set  system  clock (settimeofday(2), stime(2), adjtimex(2)); set real-time (hardware) clock.<br>
允许改变系统时钟

Test:<br>
普通用户修改系统时钟

```
#赋能前，普通用户修改系统时钟失败
[fjl@Redhat ~]$ date +%Y:%m:%d
2015:11:16
[fjl@Redhat ~]$ date -s 2015-12-12
date: cannot set date: Operation not permitted	//失败
Sat Dec 12 00:00:00 CST 2015

#赋能后，普通用户成功修改系统时钟
[root@Redhat ~]# setcap CAP_SYS_TIME=eip /bin/date
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ date -s 2015-12-12
Sat Dec 12 00:00:00 CST 2015		
[fjl@Redhat ~]$ date +%Y:%m:%d
2015:12:12    //修改成功
[fjl@Redhat ~]$ date -s 2015-11-12
Thu Nov 12 00:00:00 CST 2015
[fjl@Redhat ~]$ date +%Y:%m:%d    
2015:11:12    //修改成功
```

#### 34)CAP_SYS_TTY_CONFIG
Capability:<br>
Use vhangup(2).<br>
允许配置TTY设备

Test:

```
#hanguptty程序代码
[root@Redhat ~]# cat hanguptty.c 
#include <unistd.h>
void main()
{
        vhangup();	//挂起当前tty
}

#赋能前，vhangup调用失败
[fjl@Redhat ~]$ /tmp/hanguptty
[fjl@Redhat ~]$ echo $?
255		//失败

#赋能后，vhangup调用成功，当前tty被挂起，连接断开
[root@Redhat ~]# setcap CAP_SYS_TTY_CONFIG=eip /tmp/hanguptty
[root@Redhat ~]# getcap /tmp/hanguptty
/tmp/hanguptty = cap_sys_tty_config+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ /tmp/hanguptty
Connection to 192.168.1.20 closed.	//tty被挂起，连接断开
```

#### 35)CAP_SYSLOG
Perform privileged  syslog(2)  operations.   See  syslog(2)  for information on which operations require privilege.
