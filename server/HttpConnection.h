#include <arpa/inet.h>
#include "Buffer.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;
class HttpConnection
{
private:
    /* data */
    int cli_fd;
    sockaddr_in cli_addr;
    
public:
    HttpConnection(/* args */);
    ~HttpConnection();
    void initHttpConnc(int cli_fd,sockaddr_in cli_addr);//用来初始化这个connection对象的一些属性
    int getFd();
};


