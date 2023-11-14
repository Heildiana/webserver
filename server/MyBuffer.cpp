#include "MyBuffer.h"
#include "MyBuffer.h"


MyBuffer::MyBuffer(/* args */)
{
    this->cur_idx = 0;
    this->buffer_ptr = new std::vector<char>(128);
}

MyBuffer::~MyBuffer()
{
    delete buffer_ptr;
}

void MyBuffer::readFd(int fd)
{
    int len;
    int read_pos = 0;
    while (1)
    {
        len = read(fd,buffer_ptr->data()+read_pos,buffer_ptr->capacity()-read_pos);
        read_pos+=len;
        if(len<=0){
            break;
        }
        buffer_ptr->resize(buffer_ptr->capacity()*2);
        // std::cout<<"capasity: "<<buff_v.capacity()<<std::endl;
    }
    line_begin = buffer_ptr->begin();
}

void MyBuffer::writeFd()
{
}

std::string MyBuffer::nextLine()
{
    // std::cout<<"next line"<<std::endl;
    char CRTF[]= "\r\n";
    auto line_end = std::search(line_begin,(*buffer_ptr).end(),CRTF,CRTF+2);
    std::string cur_line(line_begin,line_end);
    line_begin = line_end+2;
    // std::cout<<"curline:"<<std::endl;
    return cur_line;

    // auto line_end = std::search((*buffer_ptr).begin()+cur_idx,(*buffer_ptr).end(),CRTF,CRTF+2);
    // std::string cur_line((*buffer_ptr).begin()+cur_idx,line_end);
    // cur_idx = line_end-buffer_ptr->begin()+2;
    // std::cout<<"curline:"<<std::endl;
    // return cur_line;
}

int MyBuffer::getBufferSize()
{
    return buffer_ptr->size();
}

void MyBuffer::printContent()
{
    std::cout<<"buffer content:"<<std::endl;
    for(auto ch:*buffer_ptr){
        std::cout<<ch;
    }
    std::cout<<std::endl;
}

