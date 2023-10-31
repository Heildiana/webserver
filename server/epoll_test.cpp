#include "MyEpoll.h"
#include "MyEpoll.cpp"
#include "mySocket.h"
#include <iostream>

int main()
{
    mySocket sock;
    MyEpoll epoll_;
    int listen_fd = sock.creatListenSock(1316);
    std::cout<<"ls fd"<<std::endl;
    
    
    epoll_.addFd(listen_fd,EPOLLIN);
    epoll_.wait();
    std::cout<<"wait return"<<std::endl;
    return 0;


    // int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    // sockaddr_in ls_sock;
    // ls_sock.sin_addr.s_addr = INADDR_ANY;
    // ls_sock.sin_family = AF_INET;
    // ls_sock.sin_port = ntohs(1316);
    // bind(listen_fd,(sockaddr*)&ls_sock,sizeof(ls_sock));
    // listen(listen_fd,32);

    //创建epoll
    int epfd = epoll_create(666);

    epoll_event ls_event;//ls 加入监听
    ls_event.data.fd = listen_fd;
    ls_event.events = EPOLLIN;

    epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&ls_event);

    epoll_event events[128];

    std::cout<<"wait "<<std::endl;
    epoll_wait(epfd,events,128,-1);
    std::cout<<"wait return"<<std::endl;



    // //socket 创建监听端口
    // int lfd = socket(PF_INET,SOCK_STREAM,0);
    // if(lfd==-1){
    //     perror("socket");
    // }
    // //初始化server的struct
    // struct sockaddr_in servAddr;
    // servAddr.sin_addr.s_addr = INADDR_ANY;
    // servAddr.sin_family = AF_INET;
    // servAddr.sin_port = ntohs(1316);

    // //bind 暴露对外端口
    // bind(lfd,(struct sockaddr*)&servAddr,sizeof(servAddr));
    
    // //确定listen队列大小
    // listen(lfd,64);

    // //创建epoll实例
    // int epfd = epoll_create(666);//这个值随便的
    // //用ctl将lfd加到rbr里面
    // typedef struct epoll_event epoll_event;
    // epoll_event epev;
    // epev.events = EPOLLIN;
    // epev.data.fd = lfd;
    // epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&epev);

    // epoll_event epevs[1024];
    
    // printf("epoll wait\n");
    // int ret = epoll_wait(epfd,epevs,1024,-1);//阻塞,返回的epevs里面就全是变化了的fd
    // printf("wait return\n");

}
