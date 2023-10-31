#include "MyEpoll.h"
#include "../server/mySocket.h"
#include <iostream>

int main(int argc, char const *argv[])
{
    mySocket sock;
    MyEpoll epoll_;
    int listen_fd = sock.creatListenSock(1316);
    
    epoll_.addFd(listen_fd,EPOLLIN);
    epoll_.wait();
    std::cout<<"wait return"<<std::endl;
    return 0;
}
