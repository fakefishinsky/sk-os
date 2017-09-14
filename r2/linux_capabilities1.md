## Linux Capabilities
系统上的很多操作只能以root权限执行，某些情况下业务要实现一些高权限的功能，使用普通用户运行程序可能无法达到预期的功能，
<br>
而如果直接使用root用户运行程序，则会获得root用户的全部权限，一旦程序存在漏洞被利用，则对系统的危害很大。
<br>
利用Linux Capabilities可以使程序获得其需要的权限，而不必直接使用root用户运行程序，满足最小权限原则。
<br>
![capability-logo](images/capability-logo.png)
<br>
详细信息请参考[man-page-capabilities](http://man7.org/linux/man-pages/man7/capabilities.7.html)。
### Capabilitiy Description
For the purpose of performing permission checks, traditional UNIX implementations distinguish two  categories  of  processes:  privileged processes  (whose  effective  user ID is 0, referred to as superuser or root), and unprivileged processes (whose  effective  UID  is  nonzero).
<br>
Privileged processes bypass all kernel permission checks, while unprivileged processes are subject to full permission checking based  on  the  process's  credentials (usually: effective UID, effective GID, and supplementary group list).
<br>
Starting with kernel 2.2, Linux divides  the  privileges  traditionally associated  with  superuser into distinct units, known as capabilities, which can be independently enabled and disabled.   Capabilities  are  a  per-thread attribute.

### Thread Capability Sets
Each thread has three capability sets containing zero or  more  of  the above capabilities:
<br>
Permitted(p):
<br>
This  is a limiting superset for the effective capabilities that the thread may assume.  It is also a limiting superset  for  the capabilities  that  may  be  added  to  the inheritable set by a              thread that does not have  the  CAP_SETPCAP  capability  in  its effective set.
<br>
If  a  thread  drops a capability from its permitted set, it can never reacquire that capability (unless it execve(2)s  either  a set-user-ID-root  program,  or  a  program whose associated file  capabilities grant that capability).
<br>
Inheritable(i):
<br>
This is a set of capabilities preserved across an execve(2).  It provides a mechanism for a process to assign capabilities to the permitted set of the new program during an execve(2).
<br>
Effective(e):
<br>
This is the set of capabilities used by the  kernel  to  perform permission checks for the thread.
<br>
![thread-capability](images/thread-capabilities.png)

### Capabilitiy List
The following list shows the capabilities implemented on Linux, and the operations or behaviors that each capability permits:

#### 1)CAP_AUDIT_CONTROL
Enable and  disable  kernel  auditing;  change  auditing  filter  rules; retrieve auditing status and filtering rules.

#### 2)CAP_AUDIT_WRITE
Write records to kernel auditing log.

#### 3)CAP_CHOWN
Capability:
<br>
Make arbitrary changes to file UIDs and GIDs (see chown(2)).
<br>
允许改变文件的所有权

Test:
<br>
使用普通用户修改任意文件的属主/属组。
```
#目标文件/etc/shadow
[root@Redhat ~]# ll /etc/shadow
----------. 1 root root 1318 Nov 14 02:39 /etc/shadow

#赋予CAP_CHOWN能力之前，普通用户fjl修改/etc/shadow文件的所有权
[fjl@Redhat ~]$ whoami
fjl
[fjl@Redhat ~]$ chown fjl:fjl /etc/shadow
chown: changing ownership of `/etc/shadow': Operation not permitted  //没有权限，执行失败

#给/bin/chown程序赋予CAP_CHOWN能力
[root@Redhat ~]# setcap CAP_CHOWN=ep /bin/chown
(SUSE系统上安装libcap-progs包；Redhat系统上安装libcap包)
[root@Redhat ~]# getcap /bin/chown
/bin/chown = cap_chown+ep
[root@Redhat ~]# ll /bin/chown
-rwxr-xr-x. 1 root root 57464 Jul 16  2014 /bin/chown	//没有SUID/SGID位

#赋予CAP_CHOWN能力后，普通用户fjl修改/etc/shadow文件的所有权
[fjl@Redhat ~]$ whoami
fjl
[fjl@Redhat ~]$ chown fjl:fjl /etc/shadow
[fjl@Redhat ~]$ ll /etc/shadow
----------. 1 fjl fjl 1318 Nov 14 02:39 /etc/shadow    //修改成功

#移除能力
[root@Redhat ~]# setcap -r /bin/chown
```
#### 4)CAP_DAC_OVERRIDE
Capability:
<br>
Bypass file read, write, and execute permission checks.  (DAC is an abbreviation of “discretionary access control".)
<br>
忽略文件的所有DAC访问控制（vi/vim/echo/sed/awk等等）

Test:
<br>
使用普通用户修改任意文件的内容
```
#目标文件/etc/shadow
[root@Redhat ~]# ll /etc/shadow
----------. 1 root root 1325 Nov 14 22:23 /etc/shadow

#给/usr/bin/vim程序赋予CAP_DAC_OVERRIDE能力
[root@Redhat ~]# setcap CAP_DAC_OVERRIDE=eip /usr/bin/vim
[root@Redhat ~]# getcap /usr/bin/vim
/usr/bin/vim = cap_dac_override+eip

#普通用户fjl修改/etc/shadow文件内容
[fjl@Redhat ~]$ whoami
fjl
[fjl@Redhat ~]$ vi /etc/shadow
nobody:!*:15937:7:99999:7:::
fjl:$6$xxx:16752:7
:90:7:::
oracle::::::::
"/etc/shadow" 33L, 1333C written
```
#### 5)CAP_DAC_READ_SEARCH
Capability:
<br>
Bypass file read permission checks and directory read  and  execute permission checks.
<br>
忽略所有对读、搜索操作的限制

Test:
<br>
使用普通用户查看系统任意文件内容

```
#目标文件/etc/shadow
[root@Redhat ~]# ll /etc/shadow
----------. 1 root root 1325 Nov 14 22:23 /etc/shadow

#给/bin/cat程序赋予CAP_DAC_READ_SEARCH能力
[root@Redhat ~]# setcap CAP_DAC_READ_SEARCH=eip /bin/cat
[root@Redhat ~]# getcap /bin/cat
/bin/cat = cap_dac_read_search+eip

#普通用户fjl查看/etc/shadow文件内容
[fjl@Redhat ~]$ cat /etc/shadow | head -n2
root:$6$xxxx:16705:7:99999:7:::
bin:!*:15937:7:99999:7:::
```

#### 6)CAP_FOWNER
Capability:
* Bypass  permission  checks on operations that normally require the file system UID of the process to match  the  UID  of  the file  (e.g.,  chmod(2),  utime(2)), excluding those operations covered by CAP_DAC_OVERRIDE and CAP_DAC_READ_SEARCH;
* set extended file  attributes  (see  chattr(1))  on  arbitrary files;
* set Access Control Lists (ACLs) on arbitrary files;
* ignore directory sticky bit on file deletion;
* specify O_NOATIME for arbitrary files in open(2) and fcntl(2).
<br>
以最后操作的UID，覆盖文件的先前的UID

Test:
<br>
普通用户修改任意文件权限

```
#目标文件/etc/shadow
[root@Redhat ~]# ll /etc/shadow
----------. 1 root root 1325 Nov 14 22:23 /etc/shadow

#给/bin/chmod赋予CAP_FOWNER能力
[root@Redhat ~]# setcap CAP_FOWNER=eip /bin/chmod

#普通用户fjl修改/etc/shadow文件权限为644
[fjl@Redhat ~]$ chmod 644 /etc/shadow
[fjl@Redhat ~]$ ll /etc/shadow
-rw-r--r--. 1 root root 1318 Nov 15 01:07 /etc/shadow
```
#### 7)CAP_FSETID
Capability: 
<br>
Don't  clear set-user-ID and set-group-ID permission bits when a file is modified; set the set-group-ID bit for a file whose  GID does  not match the file system or any of the supplementary GIDs of the calling process. Establish leases on arbitrary files (see fcntl(2)).
<br>
确保在文件被修改后不修改setuid/setgid位

Test:

```
#没有CAP_FSETID能力时，文件修改后suid/sgid位被移除：
[root@Redhat ~]# touch /tmp/test
[root@Redhat ~]# chmod 6777 /tmp/test
[root@Redhat ~]# ll /tmp/test        
-rwsrwsrwx. 1 root root 3 Nov 15 01:41 /tmp/test
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ vi /tmp/test
aa                                                                                                         
"/tmp/test" 1L, 3C written                                                                  
 [root@Redhat ~]# ll /tmp/test
-rwxrwxrwx. 1 root root 3 Nov 15 01:41 /tmp/test	//suid/sgid位已经被移除

#有CAP_FSETID能力时，文件修改后suid/sgid位不变
[root@Redhat ~]# chmod 6777 /tmp/test
[root@Redhat ~]# ll /tmp/test        
-rwsrwsrwx. 1 root root 3 Nov 15 01:41 /tmp/test
[root@Redhat ~]# setcap CAP_FSETID=eip /usr/bin/vim
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ vi /tmp/test
aaaaaaaa                                                                                                          
"/tmp/test" 1L, 9C written                                                                  
 [root@Redhat ~]# ll /tmp/test
-rwsrwsrwx. 1 root root 9 Nov 15 01:47 /tmp/test	//suid/sgid位保持不变
```

#### 8)CAP_IPC_LOCK
Capability: 
<br>
Lock memory (mlock(2), mlockall(2), mmap(2), shmctl(2)).
<br>
允许锁定内存片段，root和普通用户都可以用mlock来锁定内存，但是root不受ulimit下的锁定内存大小限制，而普通用户会受到影响。

mlocktest程序代码:

```
[root@Redhat ~]# cat mlocktest.c 
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
void main()
{
        int array[2048];
        if( mlock((const void *)array,sizeof(array)) == -1 )	//锁定内存
        {
                perror("mlock error");
                return;
        }
        printf("success to lock stack mem at: %p, len=%zd\n",array,sizeof(array));
        if( munlock((const void *)array,sizeof(array)) == -1 )	//解锁
        {
                perror("munlock error");
                return;
        }
        printf("success to unlock stack mem at: %p, len=%zd\n",array,sizeof(array));
}
```
Test:

```
[fjl@Redhat ~]$ ulimit -l
64		ulimit限制锁定内存大小为64K
[fjl@Redhat ~]$ /tmp/mlocktest
success to lock stack mem at: 0x7fff74a46e50, len=8192	len=8192B<64K，锁定成功
success to unlock stack mem at: 0x7fff74a46e50, len=8192
[fjl@Redhat ~]$ ulimit -l 1	设置锁定内存大小限制为1K
[fjl@Redhat ~]$ ulimit -l     
1		
[fjl@Redhat ~]$ /tmp/mlocktest
mlock error: Cannot allocate memory		//赋能前，普通用户不能突破ulimit限制
[fjl@Redhat ~]$ exit
logout
[root@Redhat ~]# setcap CAP_IPC_LOCK=eip /tmp/mlocktest
[root@Redhat ~]# getcap /tmp/mlocktest
/tmp/mlocktest = cap_ipc_lock+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ ulimit -l
64
[fjl@Redhat ~]$ ulimit -l 1
[fjl@Redhat ~]$ ulimit -l
1
[fjl@Redhat ~]$ /tmp/mlocktest
success to lock stack mem at: 0x7fff661aea30, len=8192	len=8192B > 1K，  //锁定成功，突破ulimit限制
success to unlock stack mem at: 0x7fff661aea30, len=8192
```

#### 9)CAP_IPC_OWNER
Capability: 
<br>
Bypass permission checks for operations on System V IPC objects.
<br>
忽略IPC所有权检查，读写（不能删除）任意共享内存（绕过共享内存的权限控制）

Test:
<br>
普通用户修改root用户创建的共享内存
<br>
ipcownertest程序代码:
```
[root@Redhat ~]# cat ipcownertest.c 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

void error_out(const char *msg)
{
        perror(msg);
        exit(EXIT_FAILURE);
}

int main (int argc, char *argv[])
{
        key_t mykey = 12345678;

        const size_t region_size = sysconf(_SC_PAGE_SIZE);
        int smid = shmget(mykey, region_size, IPC_CREAT|0600);	//root创建共享内存
        if(smid == -1)
                error_out("shmget");

        void *ptr;
        ptr = shmat(smid, NULL, 0);
        if (ptr == (void *) -1)
                error_out("shmat");
        u_long *d = (u_long *)ptr;
        *d = 0x11111111;		//设置数据为0x11111111
        printf("ipc mem %#lx\n", *(u_long *)ptr);

        return 0;
}
```
测试:

```
[root@Redhat ~]# /tmp/ipcownertest
ipc mem 0x11111111	root用户创建共享内存，数据为0x11111111
[root@Redhat ~]# ipcs -m
------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00bc614e 0          root       600        4096       0                       
修改ipcownertest代码，将0x11111111改为0x22222222
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ /tmp/ipcownertest2 
shmget: Permission denied		//赋能前，普通用户修改root创建的共享内存失败
[fjl@Redhat ~]$ exit
logout
[root@Redhat ~]# setcap CAP_IPC_OWNER=eip /tmp/ipcownertest2
[root@Redhat ~]# getcap /tmp/ipcownertest2
/tmp/ipcownertest2 = cap_ipc_owner+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ /tmp/ipcownertest2
ipc mem 0x22222222		//赋能后，普通用户修改root创建的共享内存成功
```
#### 10)CAP_KILL
Capability: 
<br>
Bypass permission checks  for  sending  signals  (see  kill(2)). This includes use of the ioctl(2) KDSIGACCEPT operation.
<br>
允许对不属于自己的进程发送信号

Test:

```
#赋能前普通用户fjl尝试kill掉root用户的进程，失败
[fjl@Redhat ~]$ ps aux | grep pass | grep -v grep
root     12217  0.0  0.1 161876  2048 pts/0    S+   03:51   0:00 passwd lj
[fjl@Redhat ~]$ kill -9 12217
-bash: kill: (12217) - Operation not permitted

#赋能后普通用户fjl成功kill掉root用户的进程
[root@Redhat ~]# setcap CAP_KILL=eip /bin/kill
[root@Redhat ~]# getcap /bin/kill
/bin/kill = cap_kill+eip
[fjl@Redhat ~]$ ps aux | grep pass | grep -v grep
root     12356  0.0  0.1 161876  2056 pts/0    S+   03:58   0:00 passwd lj
[fjl@Redhat ~]$ /bin/kill 12356
[root@Redhat ~]# passwd lj
Changing password for user lj.
New password: Terminated			//root用户的进程已经被终止掉
```

#### 11)CAP_LEASE
Establish leases on arbitrary files (see fcntl(2)).

#### 12)CAP_LINUX_IMMUTABLE
Capability: 
<br>
Set  the  FS_APPEND_FL  and  FS_IMMUTABLE_FL  i-node  flags (see chattr(1)).
<br>
允许修改文件的不可修改(IMMUTABLE)和只添加(APPEND-ONLY)属性

Test:

```
#赋能前，普通用户无权对文件设置IMMUTABLE / APPEND-ONLY属性
[fjl@Redhat ~]$ ll test
-rw-------. 1 fjl fjl 0 Nov 15 20:27 test
[fjl@Redhat ~]$ chattr +a test
chattr: Operation not permitted while setting flags on test
[fjl@Redhat ~]$ chattr +i test
chattr: Operation not permitted while setting flags on test

#赋能后，普通用户可以对文件设置IMMUTABLE / APPEND-ONLY属性
[root@Redhat ~]# setcap CAP_LINUX_IMMUTABLE=eip /usr/bin/chattr
[root@Redhat ~]# getcap /usr/bin/chattr
/usr/bin/chattr = cap_linux_immutable+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ chattr +a test
[fjl@Redhat ~]$ chattr +i test
[fjl@Redhat ~]$ lsattr test
----ia-------e- test
```

#### 13)CAP_MAC_ADMIN
Capability: 
<br>
Override Mandatory Access Control (MAC).   Implemented  for  the Smack Linux Security Module (LSM).

#### 14)CAP_MAC_OVERRIDE
Capability: 
<br>
Allow  MAC  configuration or state changes.  Implemented for the Smack LSM.

#### 15)CAP_MKNOD
Capability: 
<br>
Create special files using mknod(2).
<br>
允许使用mknod系统调用创建特殊文件，如块设备/字符设备等

Test:

```
#赋能前，普通用户fjl无权创建块设备/字符设备
[fjl@Redhat ~]$ mknod bdev b 1 1
mknod: `bdev': Operation not permitted
[fjl@Redhat ~]$ mknod cdev c 1 1
mknod: `cdev': Operation not permitted

#赋能后，普通用户fjl可以创建块设备/字符设备
[root@Redhat ~]# setcap CAP_MKNOD=eip /bin/mknod
[root@Redhat ~]# getcap /bin/mknod
/bin/mknod = cap_mknod+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ mknod bdev b 1 1
[fjl@Redhat ~]$ mknod cdev c 1 1
[fjl@Redhat ~]$ ll
brw-------. 1 fjl  fjl  1, 1 Nov 15 21:16 bdev
crw-------. 1 fjl  fjl  1, 1 Nov 15 21:16 cdev
```

#### 16)CAP_NET_ADMIN
Capability: 
<br>
Perform various network-related operations (e.g., setting privileged  socket options, enabling multicasting, interface configuration, modifying routing tables).
<br>
允许执行网络管理任务:接口、防火墙和路由等

Test:
<br>
普通用户管理主机接口

```
#赋能前，普通用户无权管理接口
[fjl@Redhat ~]$ ifconfig lo down
SIOCSIFFLAGS: Permission denied	

#赋能后，普通用户可以管理接口	
[root@Redhat ~]# setcap CAP_NET_ADMIN=eip /sbin/ifconfig
[root@Redhat ~]# getcap /sbin/ifconfig
/sbin/ifconfig = cap_net_admin+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ ifconfig lo down
[fjl@Redhat ~]$ ifconfig
eth0      Link encap:Ethernet  HWaddr 00:x:x:x:x:x  
          inet addr:10.X.X.X  Bcast:10.X.X.X  Mask:255.255.255.0
[fjl@Redhat ~]$ ifconfig lo up
[fjl@Redhat ~]$ ifconfig
eth0      Link encap:Ethernet  HWaddr 00:x:x:x:x:x   
          inet addr:10.X.X.X  Bcast:10.X.X.X  Mask:255.255.255.0
lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
```
#### 17)CAP_NET_BIND_SERVICE
Capability: 
<br>
Bind a socket to Internet domain privileged ports (port  numbers less than 1024).
<br>
允许绑定到小于1024的端口

Test:
<br>
使用普通用户fjl绑定小于1024的端口（如100）

```
#赋能前，绑定端口100失败
[fjl@Redhat ~]$ nc -l 100
nc: Permission denied

#赋能后，成功绑定到100端口
[root@Redhat ~]# setcap CAP_NET_BIND_SERVICE=eip /usr/bin/nc
[root@Redhat ~]# getcap /usr/bin/nc
/usr/bin/nc = cap_net_bind_service+eip
[root@Redhat ~]# su - fjl
[fjl@Redhat ~]$ nc -l 100
Hello
[root@Redhat ~]# lsof -i:100
COMMAND   PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
nc      16243  fjl    3u  IPv4 106830      0t0  TCP *:newacct (LISTEN)
[root@Redhat ~]# nc 127.0.0.1 100		客户端连接成功
Hello
```

#### 18)CAP_NET_BROADCAST
 (Unused)  Make socket broadcasts, and listen to multicasts.
<br>
允许网络广播和多播
