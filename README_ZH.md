# WiFi-Portal

![](https://img.shields.io/badge/license-GPLV3-brightgreen.svg?style=plastic "License")

WiFi-Portal是一个非常高效的portal认证解决方案。它参考了wifidog和apfree_wifidog，
是一个全新的portal认证解决方案，代码简洁，框架清晰。与wifidog和apfree_wifidog
不同的是，wifi-portal通过编写内核模块实现认证管理，而不是通过iptables创建防火墙规则。

## 特性:
* 采用高效的事件库libev
* 通过编写内核模块实现认证管理，而不是通过iptables创建防火墙规则
* 支持HTTPS
* SSL库可以选择openssl或者mbedtls
* 支持远程控制和配置

# 贡献代码

Evmongoose使用github托管其源代码，贡献代码使用github的PR(Pull Request)的流程，十分的强大与便利:

1. [创建 Issue](https://github.com/zhaojh329/wifi-portal/issues/new) - 对于较大的
	改动(如新功能，大型重构等)最好先开issue讨论一下，较小的improvement(如文档改进，bugfix等)直接发PR即可
2. Fork [wifi-portal](https://github.com/zhaojh329/wifi-portal) - 点击右上角**Fork**按钮
3. Clone你自己的fork: ```git clone https://github.com/$userid/wifi-portal.git```
4. 创建dev分支，在**dev**修改并将修改push到你的fork上
5. 创建从你的fork的**dev**分支到主项目的**dev**分支的[Pull Request] -  
	[在此](https://github.com/zhaojh329/wifi-portal)点击**Compare & pull request**
6. 等待review, 需要继续改进，或者被Merge!
	
## 感谢以下项目提供帮助
* [wifidog-gateway](https://github.com/wifidog/wifidog-gateway)
* [apfree_wifidog](https://github.com/liudf0716/apfree_wifidog)
* [mongoose](https://github.com/cesanta/mongoose)
* [libev](https://github.com/kindy/libev)

# 技术交流
QQ群：153530783

# 如果该项目对您有帮助，请随手star，谢谢！