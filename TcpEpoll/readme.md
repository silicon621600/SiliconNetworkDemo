# 说明

usr/include 找不到epoll.h文件
>Mac OS X doesn't support epoll, but it does support **kqueue** which is very similar.


参考https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
和TcpSelect的内容编写

# 工具
1. telnet可以作为客户端，代替TcpSelect里的client.c
`telnet localhost 25555`

2. ubuntu tcpdump 抓包
 `tcpdump -D`网络适配器列表
 `tcpdump -i [网络适配器编号]`
 `tcpdump host localhost and tcp port 25555`
注意root权限
# 问题

1. 客户端`Connection reset by peer`
抓包分析，服务端发送RST
client.c 原先的代码会把字符串末的'\0'也发过去，导致测试时候我以为发送了10个字符结果发送了11个，然后最后一个未被读到，
所以服务端发送RST

>一个易犯错误：为了使用epoll，而把socket设置为非阻塞，那么接收函数和发送函数和之前的有了不同，
>发送也要改为循环发送，接收注意一般情况实际读取的数据也可能小于buf

