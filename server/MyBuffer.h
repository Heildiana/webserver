#ifndef MY_BUFFER
#define MY_BUFFER

#include <vector>
#include <unistd.h>
#include <algorithm>//search
#include "header.h"

class MyBuffer
{
private:
    /* data */
    std::vector<char>*buffer_ptr;//client的报文在这里

    int cur_idx;//当前指针位置
    std::vector<char>::iterator line_begin;//注意这个东西的赋值，不能在构造那里赋值，因为扩容后变成野指针了。

public:
    MyBuffer(/* args */);
    ~MyBuffer();
    void readFd(int fd);//数据从fd->vector
    void writeFd();

    //返回缓冲区中的下一行
    std::string nextLine();

    //返回当前buffer内容size
    int getBufferSize();

    void printContent();
};


#endif


