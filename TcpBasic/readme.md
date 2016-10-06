# 简单的tcp代码实现

1. client.c server.c
仅仅是照着书上敲的tcp样例程序,熟悉linuxC的tcp编写.
client 从server不停地获取时间字符串
	
```
gcc client.c -o client
gcc server.c -o server
./server
```
新开窗口

```
./client localhost
```
