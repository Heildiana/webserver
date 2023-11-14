#include "HttpRequest.h"

HttpRequest::HttpRequest(/* args */)
{
    init();
}



void HttpRequest::init()
{
    method_=path_=version_=body_="";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}


bool HttpRequest::parse(MyBuffer &cur_buffer)
{
    std::string cur_line = cur_buffer.nextLine();
    while (cur_line.size())
    {
        switch (state_)
        {
        case REQUEST_LINE:
        if(!parseRequestLine(cur_line)){
            return false;
        }
        break;
        
        default:
            break;
        }
    }
    
    
    
    return false;
}

bool HttpRequest::parseRequestLine(std::string &cur_line)
{
    std::cout<<"parseRequestLine"<<std::endl;
    std::cout<<"cur_line"<<cur_line<<std::endl;
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch submatch;
    if(std::regex_match(cur_line,submatch,pattern)){
        method_ = submatch[1];
        std::cout<<"method_"<<method_<<std::endl;
        path_ = submatch[2];
        std::cout<<"path_"<<path_<<std::endl;
        version_ = submatch[3];
        std::cout<<"version_"<<version_<<std::endl;
        state_ = HEADERS;
        return true;
    }
    return false;
}

void HttpRequest::parsePath()
{
    if(path_=="/"){
        path_ = "index.html";
    }
}
