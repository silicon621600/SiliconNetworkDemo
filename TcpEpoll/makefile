server : TcpServer.so TcpServerMain.o
	g++  TcpServerMain.o TcpServer.so   -o server -lpthread
TcpServer.so : tcp_server_config.h TcpServer.h TcpServer.cpp
	g++  -c  TcpServer.cpp -o TcpServer.so -lpthread
TcpServerMain.o : TcpServer.h TcpServerMain.cpp
	g++ -c TcpServerMain.cpp -o TcpServerMain.o
