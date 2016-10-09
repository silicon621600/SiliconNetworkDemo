# 说明
GNU文件夹的代码来自GNU的文档,里面有关于select的使用,做了一些修改使得能在我的本地跑起来
The GNU C Library[http://www.gnu.org/software/libc/manual/html_node/index.html#SEC_Contents]
client.c[http://www.gnu.org/software/libc/manual/html_node/Byte-Stream-Example.html#Byte-Stream-Example]
server.c[http://www.gnu.org/software/libc/manual/html_node/Server-Example.html#Server-Example]

虽然用了select,但是功能上只是发送一个字符串到server端


但是如果想要返回数据给客户端,select配合上多线程才有意义,所以有当前目录下的代码
代码大部分来自[http://www.oschina.net/code/snippet_861360_27324]
最后TcpServerMain.cpp中开启一个tcp服务,接收一个字符串,将字符串反转后返回客户端
```
 make
 ./server
```
 另一个窗口
```
 gcc client.c -o client
 ./client localhost 0000000010A123456789
 gcc -lpthread multiClient.c -o multiClient
 ./multiClient
```

# 未解决的问题
1. 发现编写时客户端忘了写接收函数,那么send函数一直阻塞????
2. 服务端recv时,先接受一个数据长度n,如果客户端发来的后面的数据长度没有n那么会阻塞服务端
3. ubuntu下 multiClient测试 系统会卡死
  但是在OSX没有这个问题
# 编码实现中的问题
1. OSX用gcc编译c++会出现,用g++或clang++
```
2 warnings generated.
Undefined symbols for architecture x86_64:
  "std::terminate()", referenced from:
      ___clang_call_terminate in TcpServer-c4b772.o
      ...
```

2. 找不到main函数
```
Undefined symbols for architecture x86_64:
  "_main", referenced from:
     implicit entry/start for main executable
ld: symbol(s) not found for architecture x86_64
```
对中间过程添加加`-c`选项

3. 经典错误,枚举的时候删除集合中元素 导致 `segmentation fault`
删除前先copy一份,或者 erase(iter++)


4. ubuntu14.04下  `对‘pthread_create’未定义的引用`
	最后链接的时候也要加-lpthread
