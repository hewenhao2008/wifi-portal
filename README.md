# WiFi-Portal([中文](https://github.com/zhaojh329/wifi-portal/blob/master/README_ZH.md))

![](https://img.shields.io/badge/license-GPL_2-green.svg "License")

WiFi-Portal is a very efficient captive portal solution for wireless
router which with embeded linux(LEDE/Openwrt) system. It's referenced wifidog
and apfree_wifidog, and it's a whole new one captive portal solution. Unlike
wifidog and apfree_wifidog, wifi-portal does write kernel module to implement
authentication management instead of using iptables to create firewall rules.

## features:
* Write kernel module to implement authentication management instead of using iptables to create firewall rules
* Support for HTTP and HTTPS
* Alternative openssl and mbedtls
* Support remote control and configure