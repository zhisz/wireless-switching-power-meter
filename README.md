# wireless-switching-power-meter
基于ESP-NOW的无线开关功率计

![build_goodsbutton](https://github.com/CQUPTHXC/wireless-switching-power-meter/actions/workflows/build_goodsbutton.yml/badge.svg)  ![build_stopbutton](https://github.com/CQUPTHXC/wireless-switching-power-meter/actions/workflows/build_stopbutton.yml/badge.svg)  ![build_Wireless_power_meter](https://github.com/CQUPTHXC/wireless-switching-power-meter/actions/workflows/build_Wireless_power_meter.yml/badge.svg)
# 项目介绍
https://oshwhub.com/qingmeijiupiao/wireless-switching-power-meter
# 开发框架
本项目采用PlatformIO IDE + Arduino 框架开发
使用vscode安装PlatformIO后打开代码文件夹即可编译运行
注意不可直接打开根目录，只能打开项目文件夹否则PlatformIO无法识别

# 文件介绍
- Wireless_power_meter ：功率计本体代码
- goodsbutton ：mini开关代码
- stopbutton ：急停开关代码

# 功能介绍
## 功率计本体
    短按大按钮：关闭
    长按大按钮：开启
    侧面按键：翻页
    长按左边的侧边按键：屏幕翻转180°
## 急停开关

    拍下大按钮：急停
    恢复大按钮：恢复
## mini开关
    短按：关闭
    长按2s：开启
    长按10：发送配对包

# 固件烧录
如果不想搭建开发环境也可以选择直接烧录固件

[esp下载工具链接](https://www.espressif.com/zh-hans/support/download/other-tools)

## 下载解压后打开工具

![image](https://github.com/user-attachments/assets/9eddfbf2-39a3-4faf-9804-68ef2b215977)

## 选择 ESP32-C3和USB烧录

![image](https://github.com/user-attachments/assets/dd5756dd-e65b-41c0-9ebf-b2a14caf6c09)
![image](https://github.com/user-attachments/assets/3c837e4c-2ff2-4a40-8a6a-0984921df3d5)

## 点击三个小点选择在[github release](https://github.com/CQUPTHXC/wireless-switching-power-meter/releases)处下载的.bin文件

![image](https://github.com/user-attachments/assets/69aea8b0-c27d-409c-8d56-fd5c31565108)

## 之后如图设置烧录
烧录前需要先将esp32进入烧录模式，即：按住boot按钮再让芯片启动，电脑能够显示端口就算成功

![image](https://github.com/user-attachments/assets/e8f50ed6-1cc7-48f5-b483-b88221737867)
