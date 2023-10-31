#include <arpa/inet.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

class mySocket
{
private:
    /* data */
public:
    mySocket(/* args */);
    ~mySocket();

    int creatListenSock(int port);
};

mySocket::mySocket(/* args */)
{
}

mySocket::~mySocket()
{
}

int mySocket::creatListenSock(int port)
{
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in serAddr;
    serAddr.sin_port = ntohs(port);
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = INADDR_ANY;

    bind(lfd,(sockaddr*)&serAddr,sizeof(serAddr));
    listen(lfd,32);

    return lfd;
}
