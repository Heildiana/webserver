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

void HttpConnection::readFd()
{
    read_buffer.readFd(getFd());
}

void HttpConnection::printBuffer()
{
    read_buffer.printContent();
}

MyBuffer &HttpConnection::getBuffer()
{
    return read_buffer;
    // TODO: 在此处插入 return 语句
}

bool HttpConnection::handleHttpConnection()
{
    //完成请求的解析,并且把要发送的网页准备好
    cur_request.init();
    if(read_buffer.getBufferSize()<0){
        std::cerr<<"httpconnection:buffer empty"<<std::endl;
    }else if(cur_request.parse(read_buffer)){
        //response.init
    }

    
    
    return false;
}
