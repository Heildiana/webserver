#ifndef EPOLL
#define EPOLL
#include <sys/epoll.h>//epoll
#include <vector>
#include <unordered_map>
#include <iostream>
#include "assert.h"



typedef struct epoll_event epoll_event;
class MyEpoll
{
private:
    int epoll_fd;
    std::vector<epoll_event>epoll_events;//这里面装的是被触发了的iO fd
    

public:
    MyEpoll(int events_num=1024);//创建内核的epoll对象以及用来装事件的epoll_events数组
    ~MyEpoll();

    bool addFd(int fd,uint32_t events);
    int getFd(int i);//得到触发数组里第i个fd
    bool delFd(int fd);
    bool modFd(int fd,uint32_t event);
    u_int32_t getEvent(int i);//得到触发数组里第i个事件的event

    int wait();//封装的wait函数,返回触发的event数
};

#endif
