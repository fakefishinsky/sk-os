## 合理规划分区
系统不同目录存放不同用途的文件, /tmp和/var/tmp存放系统临时文件, /etc存放系统关键配置文件, /var存放系统运行时产生的动态文件, /var/log存放系统日志, /home存放用户数据, 等等。
<br>
合理的磁盘分区可提升系统的安全性，减少单个磁盘被耗尽影响系统可用性的风险。

### 对比分析
* 分区合理
```
SLES12-2:~ # df -h
Filesystem      Size  Used Avail Use% Mounted on
devtmpfs        3.9G  8.0K  3.9G   1% /dev
tmpfs           3.9G     0  3.9G   0% /dev/shm
tmpfs           3.9G  411M  3.5G  11% /run
tmpfs           3.9G     0  3.9G   0% /sys/fs/cgroup
/dev/sda1       8.0G  5.7G  1.9G  76% /
/dev/sda2       8.0G  5.7G  1.9G  76% /boot
/dev/sda3       8.0G  5.7G  1.9G  76% /home
/dev/sda4       8.0G  5.7G  1.9G  76% /opt
/dev/sda5       8.0G  5.7G  1.9G  76% /tmp
/dev/sda6       8.0G  5.7G  1.9G  76% /var
/dev/sda7       8.0G  5.7G  1.9G  76% /var/log
/dev/sda8       8.0G  5.7G  1.9G  76% /var/log/audit
```
* 分区不合理
```
SLES11-3:~ # df -h
Filesystem      Size  Used Avail Use% Mounted on
/dev/sda2        22G   19G  2.6G  88% /
udev            3.9G  104K  3.9G   1% /dev
tmpfs           3.9G  1.9G  2.1G  48% /dev/shm
SLES11-3:~ #
```
对于系统存放动态数据的目录以及普通用户可以控制的目录建议规划单独的分区，避免动态数据不断增长或恶意用户耗尽磁盘空间，从而导致系统不可用。
<br>
对分区设置nodev,noexec,nosuid等挂载选项，可阻止恶意程序的执行，提高系统安全性。

### 如何实施
* 常用分区
<table>
<tr>
<th>挂载点</th>
<th>用途</th>
<th>建议大小 </th>
</tr>
<tr>
<td>/</td>
<td>根分区, 用于安装操作系统</td>
<td>5G ~ 10G</td>
</tr>
<tr>
<td>/boot</td>
<td>存放系统启动数据, grub、vmlinux等</td>
<td>1G</td>
</tr>
<tr>
<td>/home</td>
<td>存放用户数据</td>
<td>>10G</td>
</tr>
<tr>
<td>/opt</td>
<td>安装第三方软件, 如Oracle等</td>
<td>>10G</td>
</tr>
<tr>
<td>/tmp</td>
<td>存放系统临时文件</td>
<td>>5G ~ 10G</td>
</tr>
<tr>
<td>/var</td>
<td>系统运行时产生的动态文件</td>
<td>5G</td>
</tr>
<tr>
<td>/var/log</td>
<td>存放系统日志</td>
<td>5G</td>
</tr>
<tr>
<td>/var/log/audit</td>
<td>如果开启auditd服务, 用于存放审计日志</td>
<td>2G</td>
</tr>
</table>

* 挂载选项
<table>
<tr>
<th>选项</th>
<th>用途</th>
</tr>
<tr>
<td>nodev</td>
<td>禁止在该分区创建字符设备、块设备等特殊设备文件</td>
</tr>
<tr>
<td>noexec</td>
<td>禁止执行该分区上的可执行文件</td>
</tr>
<tr>
<td>nosuid</td>
<td>禁止执行该分区上的SUID/SGID程序</td>
</tr>
<tr>
<td>ro</td>
<td>为分区设置只读属性, 禁止对分区执行写操作</td>
</tr>
</table>

* 示例
```
SLES12-2:~ # cat /etc/fstab
tmpfs        /dev/shm           tmpfs   rw,nosuid,nodev,noexec         0  0     
# ......
/dev/sda1    /                  btrfs   defaults                       0  0
/dev/sda2    /boot              btrfs   defaults,nodev,noexec,nosuid   0  0
/dev/sda3    /home              btrfs   defaults,nodev,nosuid          0  0     
/dev/sda4    /opt               btrfs   defaults                       0  0
/dev/sda5    /tmp               btrfs   defaults,nodev,noexec,nosuid   0  0     
/dev/sda6    /var               btrfs   defaults                       0  0     
/dev/sda7    /var/log           btrfs   defaults,nodev,noexec,nosuid   0  0     
/dev/sda8    /var/log/audit     btrfs   defaults,nodev,noexec,nosuid   0  0     
/tmp         /var/tmp           none    bind                           0  0
```
最后一行表示将/var/tmp绑定到/tmp上, 因为/var/tmp与/tmp一样, 都是全局可写目录, 这样设置可以使/var/tmp获得与/tmp同样的保护, 如果单独规划了/var/tmp分区, 则可不用绑定。

### Tips
使用LVM进行磁盘管理, 可以很方便的进行分区以及扩容等。
<br>
![LVM-Manage-Disk](images/lvm_disk_partition.png)
图1: LVM磁盘管理分区过程
