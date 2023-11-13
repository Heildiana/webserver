#include "MyBuffer.h"


MyBuffer::MyBuffer(/* args */)
{
    this->buffer_ = new std::vector<char>(128);
}

MyBuffer::~MyBuffer()
{
}

void MyBuffer::readFd(int fd)
{
    int len;
    int read_pos = 0;
    while (1)
    {
        len = read(fd,buffer_->data()+read_pos,buffer_->capacity()-read_pos);
        read_pos+=len;
        if(len<=0){
            break;
        }
        buffer_->resize(buffer_->capacity()*2);
        // std::cout<<"capasity: "<<buff_v.capacity()<<std::endl;
    }
}

void MyBuffer::writeFd()
{
}

void MyBuffer::printContent()
{
    std::cout<<"buffer content:"<<std::endl;
    for(auto ch:*buffer_){
        std::cout<<ch;
    }
    std::cout<<std::endl;
}
