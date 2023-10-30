#include "HttpConnection.h"


HttpConnection::HttpConnection(/* args */)
{
}

HttpConnection::~HttpConnection()
{
}

void HttpConnection::initHttpConnc(int cli_fd, sockaddr_in cli_addr)
{
    this->cli_fd = cli_fd;
    this->cli_addr = cli_addr;
}

int HttpConnection::getFd()
{
    return cli_fd;
}
