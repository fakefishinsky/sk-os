## 安全启动
及时更新操作系统内核和软件版本，解决已披露的安全漏洞，降低系统被攻击的风险。
### 设置BIOS密码
设置BIOS密码，系统在启动时会要求输入该密码，正确输入后，系统才会正常启动。(对远程启动系统存在影响，一般生产环境不会设置BIOS密码)
![BIOS Password](images/bios-password.png)
### 设置GRUB密码
设置GRUB密码，系统启动时如果要修改启动参数，则必须先输入该密码，可以防止系统启动参数被恶意修改。
* init
```
SLES11-3:~ # grub-md5-crypt    //先生成一个加密密码
Password: 
Retype password: 
$1$8F76U/$PKIehQIMD2c/OwgC1V.ag0    //加密密码
SLES11-3:~ # 
SLES11-3:~ # cat /boot/grub/menu.lst
#Modified by YaST2. Last modification on Wed Dec 21 11:27:44 CST 2016
default 0
timeout 8
password --md5 $1$8F76U/$PKIehQIMD2c/OwgC1V.ag0    //在此处设置加密密码
##YaST - generic_mbr
gfxmenu (hd0,1)/boot/message
##YaST - activate
###Don't change this comment - YaST2 identifier: Original name: linux###
title SUSE Linux Enterprise Server 11 SP3 - 3.0.76-0.11
    root (hd0,1)
    kernel /boot/vmlinuz-3.0.76-0.11-default root=/dev/sda2 resume=/dev/sda1 splash=silent crashkernel=256M-:128M showopts vga=0x314
    initrd /boot/initrd-3.0.76-0.11-default
```

* systemd
```
SUSE12-2-MinOS:~ # grub2-mkpasswd-pbkdf2    //先生成一个加密密码
Enter password: 
Reenter password: 
PBKDF2 hash of your password is grub.pbkdf2.sha512.10000.C1F7763EA7741B8CBDC1CA92549D0E6A2C14A68DC36B4D3AA2C0BDD9D0D665391972F35213727DCD66722BC2E249994D4BE7C9219DAFEF28A7A4428B3C344036.7736E6A24AC0A80D4EC8B9542513B8F3E63F1DE01FF11099F668AB8E837D11ABA4847AE12F2D9D4088CFEDE9382AB31298CFEF0DB55A3FDF984F36B538D6D931
SUSE12-2-MinOS:~ #
SUSE12-2-MinOS:~ # echo '''
cat <<EOF 
set superusers="bootusr" 
password_pbkdf2 bootusr grub.pbkdf2.sha512.10000.C1F7763EA7741B8CBDC1CA92549D0E6A2C14A68DC36B4D3AA2C0BDD9D0D665391972F35213727DCD66722BC2E249994D4BE7C9219DAFEF28A7A4428B3C344036.7736E6A24AC0A80D4EC8B9542513B8F3E63F1DE01FF11099F668AB8E837D11ABA4847AE12F2D9D4088CFEDE9382AB31298CFEF0DB55A3FDF984F36B538D6D931
EOF
''' >> /etc/grub.d/00_header    //在/etc/grub.d/00_header 文件末尾追加配置
SUSE12-2-MinOS:~ #
SUSE12-2-MinOS:~ # grub2-mkconfig -o /boot/grub2/grub.cfg    //重新生成grub配置文件
SUSE12-2-MinOS:~ #
```

### 系统运行级别
系统运行级别如下，服务器一般运行在level 3，个人桌面电脑运行在level 5（比level 3多了图形化界面功能）。

| runlevel | state | 
|------|------|
| 0 | System halt(Do not use this for initdefault!) |
| 1 | Single user mode |
| 2 | Local multiuser without remote network (e.g. NFS) |
| 3 | Full multiuser with network |
| 4 | Not used |
| 5 | Full multiuser with network and xdm |
| 6 | System reboot(Do not use this for initdefault!) |

设置系统运行级别(注意需重启系统才能生效):
* init
获取当前系统运行级别:
```
SLES11-3:~ # runlevel
N 5    //当前运行级别为level 5
SLES11-3:~ #
```
若要调整运行级别，修改`/etc/inittab`文件，设置:
```
# The default runlevel is defined here
id:3:initdefault:
```

* systemd
获取当前系统运行级别:
```
NKG1000115469:~ # systemctl get-default
graphical.target    //当前运行级别为graphical, 即level 5
NKG1000115469:~ # ll /etc/systemd/system/default.target
lrwxrwxrwx 1 root root 40 Sep  4 14:38 /etc/systemd/system/default.target -> /usr/lib/systemd/system/graphical.target
NKG1000115469:~ #
```
若要调整运行级别，执行:
```
NKG1000115469:~ # systemctl set-default multi-user.target    //调整为multi-user, 即level 3
Removed symlink /etc/systemd/system/default.target.
Created symlink from /etc/systemd/system/default.target to /usr/lib/systemd/system/multi-user.target.
NKG1000115469:~ # 
NKG1000115469:~ # systemctl get-default
multi-user.target
NKG1000115469:~ # ll /etc/systemd/system/default.target
lrwxrwxrwx 1 root root 41 Sep  4 14:36 /etc/systemd/system/default.target -> /usr/lib/systemd/system/multi-user.target
NKG1000115469:~ #
```

### 禁用ctrl-alt-delete
Linux下组合键ctrl-alt-delete可以实现重启操作系统，对于生产服务器，需禁止该组合键，防止误操作导致系统重启，导致业务服务不可用。
* init
将`/etc/inittab`文件中包含ctrlaltdel的行注释掉:
```
# what to do when CTRL-ALT-DEL is pressed
#ca::ctrlaltdel:/sbin/shutdown -r -t 4 now    //将这行注释掉
```

* systemd
将`/usr/lib/systemd/system/ctrl-alt-del.target`链接到`/dev/null`:
```
NKG1000115469:~ # ls -l /usr/lib/systemd/system/ctrl-alt-del.target    //默认情况下是链接到reboot.target
lrwxrwxrwx. 1 root root 13 Feb  5  2017 /usr/lib/systemd/system/ctrl-alt-del.target -> reboot.target
NKG1000115469:~ # 
NKG1000115469:~ # ln -sf /dev/null /usr/lib/systemd/system/ctrl-alt-del.target   //链接到/dev/null
NKG1000115469:~ # systemctl daemon-reload
NKG1000115469:~ # 
```
* GUI
对于图形化界面，需要按照如下步骤将Logout设置的快捷键禁用掉:
Applications -> System Tools -> Settings -> Keyboard -> Shortcuts -> System
![gui-disable-ctrl-alt-delete](images/gui-disable-ctrl-alt-delete.png)

### 单用户模式下启用认证
设置在进入单用户模式时，需要进行用户认证，防止恶意用户直接使用root用户无认证进入系统。
* init
编辑`/etc/inittab`文件，设置:
```
# what to do in single-user mode
ls:S:wait:/etc/init.d/rc S
~~:S:respawn:/sbin/sulogin
```

* systemd
文件`/usr/lib/systemd/system/rescue.service`和`/usr/lib/systemd/system/emergency.service`需包含以下内容:
```
[Service]
# ......
ExecStart=-/bin/sh -c "/usr/sbin/sulogin; /usr/bin/systemctl --job-mode=fail --no-block default"
```