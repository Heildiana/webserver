//为什么要自己扩容呢?直接read到vector<char>里面不好吗?何必要自己造轮子?
#include <vector>
#include <unistd.h>
#include <fcntl.h>//open
#include <iostream>

int main(int argc, char const *argv[])
{
    //试图把文件直接read到vector<char>里面
    std::vector<char>buff_v(128);
    int fd = open("get.txt",O_RDONLY);
    fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
    
    
    int len;
    int read_pos = 0;
    while (1)
    {
        len = read(fd,buff_v.data()+read_pos,buff_v.capacity()-read_pos);
        read_pos+=len;
        if(len<=0){
            break;
        }
        
        buff_v.resize(buff_v.capacity()*2);
        std::cout<<"capasity: "<<buff_v.capacity()<<std::endl;
    }
    
    perror("read");

    std::cout<<"content: "<<std::endl<<std::endl;
    for(auto chr:buff_v){
        std::cout<<chr;
    }
    buff_v[1]=='/n';
    std::cout<<std::endl;
    
    return 0;
}
