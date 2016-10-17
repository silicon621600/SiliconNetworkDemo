# 说明

关于udp的一些小程序
## udptest1.c

```
gcc udptest1.c -o udptest1
./udptest1
```
新开窗口
```
./udptest1 localhost 8888
```
服务端和客户端代码很相似,
服务端循环地先收后发,客户端于此相反.

**recvfrom和fgets函数都是阻塞的,fgets读到\n为止**

## udptest2.c

只是把udptest1.c中客户端函数recvfrom和sendto改为 recv 和 send
在这之前需用connect函数建立连接.

如果不用connect send函数会报错`send error.: Destination address required`
