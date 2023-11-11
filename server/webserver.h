#ifndef WEBSERVER
#define WEBSERVER

#include <iostream>
#include <memory>
#include <arpa/inet.h>//socket
#include <string>
#include <unistd.h>//getcwd()
#include "assert.h"
#include <fcntl.h>//fcntl()
#include <unordered_map>

//my .h
#include "MyEpoll.h"
#include "HttpConnection.h"
#include "ThreadPool1.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

class Webserver{
private:
    int listen_fd;
    int port_;//对外暴露的端口
    int thread_num;//线程池的线程数量
    std::string res_dir;//发给客户端的资源目录

    u_int32_t listen_event = EPOLLRDHUP|EPOLLET;//需要监听的事件
    u_int32_t connect_event = EPOLLRDHUP|EPOLLET|EPOLLONESHOT;

    bool close_flag;//是否停止

    std::unique_ptr<MyEpoll>epoll_;//封装的epoll对象,里面有我需要的epoll_events数组
    std::unique_ptr<ThreadPool>thread_pool;//线程池对象
    std::unordered_map<int,HttpConnection>http_connections;//这里面是所有的http链接
    //后续的定时器要如何设计呢?

public:
    Webserver(int port,int thread_num);//初始化
    ~Webserver();

    void mainFrame();//主要工作框架

    bool initListenSocket();//初始化listen fd

    int setFdNonBlock(int fd);//设置fd的非阻塞,为了后续read别卡着

    //事件函数
    void handleListen();

    void handleClose(HttpConnection* client);

    void handleRead(HttpConnection* client);
    void onRead(HttpConnection* client);

};

#endif