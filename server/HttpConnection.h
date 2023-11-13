#include <arpa/inet.h>
#include "MyBuffer.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
class HttpConnection//用于和client一对一的对象
{
private:
    /* data */
    int cli_fd;
    sockaddr_in cli_addr;

    MyBuffer read_buffer;//从客户端获取的请求报文在这里
    MyBuffer write_buffer;
    
public:
    HttpConnection(/* args */);
    ~HttpConnection();
    void initHttpConnc(int cli_fd,sockaddr_in cli_addr);//用来初始化这个connection对象的一些属性
    int getFd();
    void readFd();
    void printBuffer();
};


