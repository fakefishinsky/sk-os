## 操作系统安装<br>
### 1.最小化安装<br>
仅安装需要使用的软件包。
#### 1.1.好处
* 减少系统潜在安全风险，减少受影响安全漏洞数量
* 减少操作系统安全扫描问题数，降低问题分析解决成本
* 降低补丁测试／维护成本
* 减少磁盘空间占用
#### 1.2.如何实施
先手工安装一个最小化的系统（满足业务功能需要），根据系统最终安装的rpm列表制作系统镜像或者系统自动化安装配置文件（ks.cfg/autoinstall.xml）。<br>
* Redhat/CentOS<br>
可以先选择minimal模式进行系统安装，之后在根据要安装的组件的依赖关系补充安装。<br>
例如，业务需要使用NFS服务，则在配置好yum源之后，使用yum命令安装即可(会自动解决依赖关系)：<br>
```shell
[root@centos1 ~]# yum install nfs-utils
...
Install  1 Package (+17 Dependent packages)
...
Complete!
[root@centos1 ~]# 
```
* SuSE

### 2.合理规划分区<br>
### 3.升级安全补丁<br>
