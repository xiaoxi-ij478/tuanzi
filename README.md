# rjsupplicant 研究计划

这是一个复刻原版锐捷验证客户端的代码, 除了修复一些小 bug.
基本都是靠反编译得来的

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
- wpa_supplicant (<https://w1.fi/releases/wpa_supplicant-0.7.3.tar.gz>)
- libpcap (<https://web.archive.org/web/20080403212941/http://www.tcpdump.org/release/libpcap-0.9.5.tar.gz>)
