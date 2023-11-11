#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(){
    int client_fd = socket(AF_INET,SOCK_STREAM,0);

    

    struct sockaddr_in server_sock;
    server_sock.sin_family = AF_INET;
    inet_pton(AF_INET,"192.168.112.102",&server_sock.sin_addr.s_addr);
    server_sock.sin_port = htons(1316);
    connect(client_fd,(sockaddr*)&server_sock,sizeof(server_sock));
    std::cout<<"connect"<<std::endl;

    //尝试内存映射
    int fd = open("get.txt",O_RDONLY);
    char buff[128];
    memset(buff,0,sizeof(buff));
    read(fd,buff,128);


    std::cout<<"write"<<std::endl;
    std::cout<<buff<<std::endl;
    int ret = write(client_fd,buff,strlen(buff));
    if(ret<0){
        perror("write");
    }
    std::cout<<"write"<<std::endl;
    // sleep(1);
    while (1)
    {
        int len = read(client_fd,buff,128);
        //set non block
        fcntl(client_fd,F_SETFL,fcntl(client_fd,F_GETFL)|O_NONBLOCK);
        std::cout<<"len:"<<len<<std::endl;
        if(len==0){
            std::cout<<"消息接收完毕"<<std::endl;
            break;
        }else if(len>0){
            std::cout<<"server message: "<<std::endl; 
            std::cout<<buff<<std::endl; 
        }else if(len<0&&errno==EAGAIN){
           std::cout<<"非阻塞 且 消息接收完毕"<<std::endl;
            break; 
        }
    }
    sleep(1);
    std::cout<<"close"<<std::endl;
    shutdown(client_fd,SHUT_RDWR);
    close(client_fd);//这个就会发送epoll rd hup讯号
    return 0;
}