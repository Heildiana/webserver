#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include "header.h"
#include "MyBuffer.h"
#include <regex>


class HttpRequest
{
public:
    HttpRequest(/* args */);
    ~HttpRequest() = default;

    enum PARSE_STATE{//解析状态
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {//
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    void init();

    //解析当前报文内容
    bool parse(MyBuffer& cur_buffer);

    //解析请求行
    bool parseRequestLine(std::string &cur_line);

    //解析要的网页文件地址
    void parsePath();


private:
    /* data */
    PARSE_STATE state_;//状态机当前的解析状态
    std::string method_;//get?
    std::string path_;//网页文件 /
    std::string version_;//http的版本号
    std::string body_;//请求体
    std::unordered_map<std::string,std::string>header_;//请求头的键值对
    std::unordered_map<std::string,std::string>post_;//method是post?
    
};


#endif