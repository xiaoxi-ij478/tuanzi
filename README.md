# 团子 (TuanZi) 锐捷验证客户端

这是一个试图复刻原版锐捷验证客户端的代码。

基本都是靠反编译得来的，外加上我自己的一些调整

## 使用的外部库

以下是原版客户端使用的库 (后面的链接是我找到的尽量贴近原版的):

- 修改过的 iniparser (<https://gitlab.com/iniparser/iniparser>)
- tinyxml (不是 tinyxml2!) (<https://sourceforge.net/projects/tinyxml>)
- d3des (<https://github.com/jeroennijhof/vncpwd/blob/master/d3des.c>)
- md5 (以及被魔改过的版本) (<https://www.rfc-editor.org/rfc/rfc1321>)
- crc16 (<https://github.com/gityf/crc>)
- rhash (魔改里面的 SHA1, WhirlPool 和 Tiger 算法) (<https://github.com/rhash/RHash>)
- ampheck (魔改里面的 RipeMD128 算法) (<https://github.com/polarina/ampheck>)
- rc4 (<https://github.com/a7t0fwa7/win32func/blob/main/crypt/rc4.cpp>)
- openssl (<https://github.com/openssl/openssl/releases/download/OpenSSL_0_9_8b/openssl-0.9.8b.tar.gz>)
- wpa_supplicant (被修改过) (<https://w1.fi/releases/wpa_supplicant-0.7.3.tar.gz>)
- libpcap (<https://web.archive.org/web/20080403212941/http://www.tcpdump.org/release/libpcap-0.9.5.tar.gz>)

## 编译方法

这个程序仅支持 Linux，Windows ~~等平台我可能会开一个新仓库来重写~~ 但其实我不会写 Windows。

首先安装 [Codeblocks](https://www.codeblocks.org/downloads/binaries/)，然后打开 tuanzi.workspace，根据你的需要选择 Debug / Release 编译目标，然后就可以运行了。

纯 Makefile 现在还没有想法。

## 什么是团子？

团子是《熊出没》里一个角色的名字。

![vlcsnap-2025-06-04-00h22m36s526.png](vlcsnap-2025-06-04-00h22m36s526.png)
