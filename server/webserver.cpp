#include "webserver.h"


Webserver::Webserver(int port,int thread_num):
port_(port),thread_num(thread_num),epoll_(new MyEpoll()),//注册epoll,返回的fd保存在epoll_里面
thread_pool(new ThreadPool(thread_num))
{
    //得到当前的工作目录路径,以便后续把文件写给客户端
    res_dir = getcwd(nullptr,128);
    assert(res_dir.size());
    res_dir += "/resources/";
    //其他后续操作
    //http的一些静态变量 user_count等

    //创建listen socket,并加入之前创建好的epoll中
    if(!initListenSocket()){
        close_flag=true;
    }
    std::cout<<"webserver ctor"<<std::endl;
}
Webserver::~Webserver()
{
}
// 初始化

bool Webserver::initListenSocket(){
    int ret;//用于后续一系列判断的临时量

    listen_fd = socket(AF_INET,SOCK_STREAM,0);//tcp 默认

    if(listen_fd<0){
        perror("socket");
    }
    //设置优雅关闭
    struct linger opt_linger = {0};
    if(setsockopt(listen_fd,SOL_SOCKET,SO_LINGER,&opt_linger,sizeof(opt_linger))<0){
        std::cerr<<"set linger error"<<std::endl;
    }
    //设置端口复用
    int opt_val = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt_val,sizeof(opt_val))<0){
        std::cerr<<"set addr reuse error"<<std::endl;
    }

    sockaddr_in listen_addr;//对外暴露的listen socket,用于后续的bind步骤
    if(port_<1024||port_>65535){
        std::cerr<<"illegal port"<<std::endl;
    }
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = ntohs(port_);
    std::cout<<"bind"<<std::endl;
    if(bind(listen_fd,(sockaddr*)&listen_addr,sizeof(listen_addr))<0){
        std::cerr<<"bind error"<<std::endl;
    }
    if(listen(listen_fd,32)<0){
        std::cerr<<"listen err"<<std::endl;
    }
    //accept
    //加到epoll里面,listenfd 非阻塞
    setFdNonBlock(listen_fd);
    epoll_->addFd(listen_fd,listen_event|EPOLLIN);//注册listen fd
    std::cout<<"add listen fd"<<std::endl;
    return true;
}


int Webserver::setFdNonBlock(int fd)
{
    assert(fd>0);
    return fcntl(fd,F_SETFL,fcntl(fd,F_GETFL)|O_NONBLOCK);
}

void Webserver::handleListen()
{
    //accept开一个新的socket用于客户端的点对点通信
    //需要接受1.这个socket的fd 2.客户端的ip,端口等信息用于回信
    //这里试试能不能自己造个轮子,搞个dll库
    sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    std::cout<<"handle listen"<<std::endl;
    int cli_fd = accept(listen_fd,(sockaddr*)&cli_addr,&len);//这里是非阻塞的,要防止一次没读完的问题
    http_connections[cli_fd].initHttpConnc(cli_fd,cli_addr);//这行是方便往回写的,自动调用默认构造函数
    epoll_->addFd(cli_fd,connect_event|EPOLLIN);//这行是核心代码
    setFdNonBlock(cli_fd);
}

void Webserver::handleClose(HttpConnection *client)
{
    //删除
    assert(client);
    epoll_->delFd(client->getFd());
    // client->closeConnect();
}

void Webserver::handleRead(HttpConnection *client)
{
    std::cout<<"handle_read"<<std::endl;
    assert(client);
    thread_pool->Submit(std::bind(&Webserver::onRead,this,client));
}

void Webserver::onRead(HttpConnection *client)
{
    assert(client);
    //这里需要把fd的内容读到httpconnect的成员里
    client->readFd();
    // char buffer[128]={0};
    // int len = 0;
    // std::cout<<"on_read"<<std::endl;

    // while (1)    
    // {
    //     len = recv(client->getFd(),buffer,128,0);
    //     if(len<0) break;
    //     std::cout<<buffer<<std::endl;//et和oneshot起作用了的,这里不断有东西读可能是浏览器在一直发请求体
    // }
    client->printBuffer();
    return;
}

void Webserver::mainFrame(){
    //创建listen socket
    //注册epoll
    //把listen fd加入
    //这两步在构造函数里完成
    //然后根据events数组的返回,分别处理不同的io事件
    if(!close_flag)
    {
        std::cout<<"server start!"<<std::endl;
    }
    while (!close_flag)
    {
        std::cout<<"epoll wait"<<std::endl;
        int events_num = epoll_->wait();//这里阻塞,等待返回触发的事件数
        std::cout<<"epoll wait return"<<std::endl;
        for(int i = 0;i<events_num;i++){
            //分别取得当前触发事件的fd和event
            int cur_fd = epoll_->getFd(i);
            u_int32_t cur_event = epoll_->getEvent(i);
            //根据fd,event的类型来决定要干嘛
            if(cur_fd==listen_fd){
                //接入客户端,生成一个新的HttpConnect对象,放在user数组里
                std::cout<<"handle listen"<<std::endl;
                handleListen();
            }else if(cur_event&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
                //err or disconnect
                assert(http_connections.count(cur_fd)>0);
                handleClose(&http_connections[cur_fd]);
            }else if(cur_event&EPOLLIN){
                //read
                assert(http_connections.count(cur_fd)>0);
                handleRead(&http_connections[cur_fd]);
            }else if(cur_event&EPOLLOUT){
                //write
            }else{
                std::cout<<"unexpected events"<<std::endl;
            }
        }
    }
}