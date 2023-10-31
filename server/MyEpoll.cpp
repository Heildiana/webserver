#include "MyEpoll.h"

MyEpoll::MyEpoll(int events_num):
epoll_fd(epoll_create(666)),epoll_events(events_num)//分别创建epoll对象和epoll_events对象
{
    assert(epoll_fd>=0&&epoll_events.size()>0);
}

MyEpoll::~MyEpoll()
{
}

bool MyEpoll::addFd(int fd,uint32_t events)
{
    if(fd<0) return false;
    epoll_event evs={0};
    evs.data.fd = fd;
    evs.events = events;
    return epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&evs)==0;
}

int MyEpoll::getFd(int i)
{
    assert(i>=0&&i<epoll_events.size());//健壮性 假设这个函数跟前面webserver的函数不是一个人写的呢?
    return epoll_events[i].data.fd;
}

bool MyEpoll::delFd(int fd)
{
    if(fd<0){
        return false;
    }
    return epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,nullptr);
}

u_int32_t MyEpoll::getEvent(int i)
{
    assert(i>=0&&i<epoll_events.size());
    return epoll_events[i].events;
}

int MyEpoll::wait()
{
    return epoll_wait(epoll_fd,&epoll_events[0],static_cast<int>(epoll_events.size()),-1);
}
