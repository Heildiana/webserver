#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string>
#include <memory.h>


void process_(char* buffer,int& epoll_fd,int client_fd);

int main(){
    //socket
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    //bind

    struct sockaddr_in server_sock;
    server_sock.sin_addr.s_addr = INADDR_ANY;
    server_sock.sin_family = AF_INET;
    server_sock.sin_port = ntohs(1316);
    bind(listen_fd,(struct sockaddr*)&server_sock,sizeof(server_sock));
    //listen
    listen(listen_fd,32);
    //accept


    //开个缓冲池
    char buffer[1024];
    memset(buffer,0,sizeof(buffer));
    //epoll
    int epoll_fd = epoll_create(333);
    epoll_event listen_event;
    listen_event.data.fd = listen_fd;
    listen_event.events = EPOLLIN|EPOLLRDHUP;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&listen_event);
    epoll_event epoll_events[128];
    while (1)
    {
        std::cout<<"epoll 阻塞"<<std::endl;
        int events_num = epoll_wait(epoll_fd,epoll_events,128,-1);
        //遍历返回的事件
        for(int i = 0;i<events_num;i++){
            uint32_t cur_event = epoll_events[i].events;
            int cur_fd = epoll_events[i].data.fd;

            if(cur_fd==listen_fd){
                std::cout<<"listen"<<std::endl;
                struct sockaddr_in client_sock;
                unsigned int len = sizeof(client_sock);
                int client_fd = accept(listen_fd,(struct sockaddr*)&client_sock,&len);
                fcntl(client_fd,F_SETFL,fcntl(client_fd,F_GETFL)|O_NONBLOCK);
                epoll_event client_event;
                client_event.data.fd = client_fd;
                client_event.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
                epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&client_event);
            }else if(cur_event&EPOLLRDHUP){
                    std::cout<<"对方已断开"<<cur_event<<std::endl;
                    //做一些回收的任务
            }else if(cur_event&EPOLLIN){//read
                int len = 0;
                while (1)
                {
                    len = read(cur_fd,buffer,sizeof(buffer));//这里只是尝试去读128个
                    //为什么这里缓存区不够大就会在头尾出现乱码?
                    //这里有个问题就是 报文太长会把前面的给刷掉
                    std::cout<<"read len:"<<len<<std::endl;
                    if(len<=0){//EAGAIN
                        break;
                    }
                }
                std::cout<<"客户报文内容:"<<std::endl;
                std::cout<<buffer<<std::endl;
                std::cout<<"read 完毕 开始处理请求"<<std::endl;
                process_(buffer,epoll_fd,cur_fd);//在这里面写好数据
            }else if(cur_event&EPOLLOUT){
                std::cout<<"write:"<<buffer<<std::endl;
                write(cur_fd,buffer,sizeof(buffer));
                epoll_event epev;
                epev.data.fd = cur_fd;
                epev.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
                std::cout<<"reset epev"<<std::endl;
                epoll_ctl(epoll_fd,EPOLL_CTL_MOD,cur_fd,&epev);//重铸一下
            }
        }
    }
}

void process_(char* buffer,int& epoll_fd,int client_fd){
    std::cout<<"解析请求"<<std::endl;
    std::string s = "this is website\n\0";
    memset(buffer,0,sizeof(buffer));
    // buffer = "this is website\n";
    strcpy(buffer,s.data());

    epoll_event epev;
    epev.data.fd = client_fd;
    epev.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP|EPOLLONESHOT;
    std::cout<<"reset epev"<<std::endl;
    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,client_fd,&epev);//重铸一下
}