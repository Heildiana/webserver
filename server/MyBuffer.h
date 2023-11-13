#include <vector>
#include <unistd.h>
#include <iostream>

class MyBuffer
{
private:
    /* data */
    std::vector<char>*buffer_;//client的报文在这里
public:
    MyBuffer(/* args */);
    ~MyBuffer();
    void readFd(int fd);//数据从fd->vector
    void writeFd();

    void printContent();
};


