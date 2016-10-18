# 说明

这个client公用于其他server,接收三个参数，
第一个域名，
第二个数据（前10个字符传递数字表示后面数据长度），
第三个端口号，没有默认25555
第四个线程数没有默认为1

其实就是向服务器发送一字符串，然后接收服务端的返回结果


#问题
1. 执行`./client localhost 00000000101234 10000`会报`unKnown localhost socket (client): Too many open files`错误程序停止
数据是特意设计成这样，使服务端认为数据还未发送完所以socket会一直存在

换成'./client 127.0.0.1 00000000101234 10000'会出现以下情况

```
...
connect (client): Connection timed out
connect (client): Connection timed out
connect (client): Connection timed out
connect (client): Connection timed out
connect (client): Connection timed out
connect (client): Connection timed out
t:-1 []
recv error: Connection reset by peer
recv data:
t:-1 []
recv error: Connection reset by peer
recv data:
t:-1 []
recv error: Connection reset by peer
recv data:
t:-1 []
recv error: Connection reset by peer
recv data:
``` 

