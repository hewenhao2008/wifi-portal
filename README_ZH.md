# WiFi-Portal

![](https://img.shields.io/badge/license-GPL_2-green.svg "License")

WiFi-Portal是一个非常高效的portal认证解决方案。它参考了wifidog和apfree_wifidog，
是一个全新的portal认证解决方案，代码简洁，框架清晰。与wifidog和apfree_wifidog
不同的是，wifi-portal通过编写内核模块实现认证管理，而不是通过iptables创建防火墙规则。

## 特性:
* 通过编写内核模块实现认证管理，而不是通过iptables创建防火墙规则
* 支持HTTP和HTTPS
* SSL库可以选择openssl或者mbedtls
* 支持远程控制和配置