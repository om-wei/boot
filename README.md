# boot
Learn u-boot on TQ2440 board

boot主要用于学习uboot，目前还不能引导系统，可以用tftp下载运行裸机(bare metal)程序

如果想对它建立初步的概念，可以看一下项目笔记https://om-wei.github.io/20220302/boot.html
  
###TODO:
目前网络没有实现数据包重发功能
定时器中断里直接调用网络接包函数，待改进

目前实现了三种命令
```
tftp filename addr
ip info | [local | server] ip
run addr
```
地址一般指定到64M SDRAM地址范围0x30000000-0x34000000，为了不覆写boot程序，地址不要在0x30000000附近，注意地址对齐。
命令中地址使用十六进制形式，不需要0x前缀。
ip配置方面，没有网关及子网掩码设置，local和server要在同一网段。

示例：
```sh
ip local 192.168.1.100  //设置开发板IP地址
ip server 192.168.1.101 //设置tftp服务器IP地址
tftp a.bin 32000000     //下载tftp服务器目录下的a.bin 到开发板SDRAM中，以0x32000000地址
tftp file 32100000      //下载file文件到0x32100000
run 32000000            //开发板从SDRAM的0x32000000开始执行，即运行a.bin程序
```
boot链接最终目标文件时，没有使用gcc而是用ld。
gcc会自动链接必要的库文件，但是boot用gcc链接会有\_start多重定义的问题。
例如vsprintf.c中使用了除法和取余运算，这是C语言支持的，gcc的libgcc.a文件有对这些运算的支持。
使用ld不会自动链接gcc的libgcc.a文件，在Makefile里需要指定它的路径，位于gcc-arm-none-eabi工具链目录下。
