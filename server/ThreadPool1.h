//单链表的版本

#include <iostream>
#include <fstream>
#include <functional>//bind
#include <unistd.h>//sleep
#include <memory>//make_shared
#include <vector>
#include <future>//packaged_task
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string.h>//memcpy()
#include <list>
#include <time.h>
#include <algorithm>//find()
#include <atomic>//atomic

#define WORKING_THREADS_LIMIT 15//working队列线程的最少数量
#define THREADS_LIMIT 1000//最多线程总数

using namespace std;

class ThreadPool{
private:
    //queue装function对象,没有返回值的任务队列
    queue<function<void()>>task_q;
    //线程数组,每个指明自己的状态
    typedef pair<thread*,atomic<bool>> thread_pair;
    list<thread_pair*>threads_;
    
    thread* daemon_;
    

    // int thread_num;//线程总数
    atomic<int> waiting_thread_num;//waiting线程数量
    atomic<int> working_thread_num;//working线程数量
    atomic<int> increase_num;//需要增加的线程数

    
    atomic<bool> stop_flag;//停止flag
    atomic<bool> reduce_flag;//缩容flag
    

    //条件变量和锁
    mutex task_mutex;//任务锁
    mutex threads_mutex;//线程锁


    condition_variable submit_cv;
    condition_variable waiting_cv;//waiting队列的条件变量
    condition_variable working_cv;//working队列的条件变量

    //需要限制任务池的最大容量吗?
    int max_task;//最大任务数,软上限




public:
    //构造线程池,初始化线程数组
    ThreadPool(int thread_num):stop_flag(false),reduce_flag(false),max_task(thread_num*2),
    waiting_thread_num(0),working_thread_num(0),increase_num(0)
    {

        //需要给出一个常驻回收线程 1 回收 2 打印当前线程数量 daemon线程

        for(int i = 0; i < thread_num; i++){
            promise<thread_pair*>pro;
            future<thread_pair*>fu = pro.get_future();
            //future 应该是不能被拷贝构造,只能传引用
            //future 怎么传到lambda表达式里面
            thread* new_thread = new thread(bind(
                    [this](future<thread_pair*>&fu){this->thread_work(fu);},move(fu)
                    )
                            );
            thread_pair* new_pair = new thread_pair(new_thread,true);
            pro.set_value(new_pair);
            threads_.emplace_back(new_pair);
            working_thread_num++;
        }
        // cout<<"ctor working_thread_num:"<<working_thread_num<<endl;
        daemon_ = new thread([this](){ this->daemon_thread_work();});
    }

    ~ThreadPool(){//join 所有的线程
        stop_flag = true;
        //设置为true后,当task队列空时,线程返回
        // cout<<"~daemon dtor"<<endl;
        daemon_->join();
        // cout<<"~daemon dtor"<<endl;
    }

    //daemon线程动作
    void daemon_thread_work(){
        while (!stop_flag){
            //什么时候扩容线程? 发现队列为满时=====这个交给submit函数做了
            //什么时候缩容线程? waiting队列太多时
            sleep(1);
            //submit函数提交了增加容量申请
            while (increase_num&&(working_thread_num+waiting_thread_num<=THREADS_LIMIT))
            {
                promise<thread_pair*>pro;
                future<thread_pair*>fu = pro.get_future();
                //future 应该是不能被拷贝构造,只能传引用
                //future 怎么传到lambda表达式里面
                thread* new_thread = new thread(bind(
                        [this](future<thread_pair*>&fu){this->thread_work(fu);},move(fu)
                        )
                                );
                thread_pair* new_pair = new thread_pair(new_thread,true);
                pro.set_value(new_pair);
                threads_.emplace_back(new_pair);
                working_thread_num++;
                max_task++;
                increase_num--;
                // cout<<"thread+1"<<endl;
                // cout<<"thread num:"<<working_thread_num+waiting_thread_num<<endl;
            }
            submit_cv.notify_all();//唤醒所有阻塞的submit函数
            //检查是否要回收线程
            if(0&&waiting_thread_num>working_thread_num)
            {
                
                // cout<<"reduce-------"<<endl;
                reduce_flag= true;
                //开始join
                waiting_cv.notify_all();
                for(auto ite = threads_.begin();ite!=threads_.end();++ite){
                    if((*ite)->second==false){//需要回收的
                        threads_.erase(ite);//队列里拿掉
                        (*ite)->first->detach();//分离线程
                        waiting_thread_num--;
                        max_task--;
                    }
                }
                reduce_flag = false;
            }
            // cout<<"===================cur thread num: "<<working_thread_num+waiting_thread_num<<endl;
        }
        //stop flag 为真 需要join所有线程
        // cout<<"daemon join"<<endl;
        {
            // cout<<"working notify"<<endl;
            working_cv.notify_all();
            // cout<<"waiting notify"<<endl;
            waiting_cv.notify_all();//这行的bug
            //程序依赖本行注释运行,没有这行注释会偶尔死锁
            
            // cout<<"joing waiting"<<endl;
            for(auto ite:threads_){
                (*ite).first->join();
            }
            // cout<<"joing waiting"<<endl;
        }
        return;
    }

    //线程动作
    //最开始在working队列里
    //试图去task_queue接任务
    //发现queue size为0
    //把自己挂进阻塞队列里面
    void thread_work(future<thread_pair*>& fu){
        thread_pair* this_thread = fu.get();
//        cout<<my_ptr<<endl;
//        sleep(2);
        while (1){
            function<void()>task;
            //阻塞,等待queue中的任务,临界资源是队列,访问任务队列的代码段需要加锁
            {
                unique_lock<mutex>task_ul(task_mutex);//对任务队列上锁
//                cout<<"th stop:"<<stop_flag<<endl;
//                cout<<"task_q size:"<<task_q.size()<<endl;
                // cout<<"task_q.size "<<task_q.size()<<endl;
                if(task_q.size()==0){//没有任务那么把自己从working挂到waiting里
                    if(stop_flag) return;//working线程出口
//                    cout<<"working to waiting"<<endl;
                    if(working_thread_num<=WORKING_THREADS_LIMIT){//如果工作线程过少,那就不进waiting队列了
                        // cout<<"+++++++++working wait"<<endl;
                        working_cv.wait(task_ul,[this](){return stop_flag||(this->task_q.size()>0);});
                        if(stop_flag) return;//working线程出口
                    }else{//working线程数量还很多
                        //如果缩容flag亮了那么就return
                        //那么缩容flag什么时候恢复呢?当缩容计数器=0的时候就恢复,在daemon那边恢复的
                        this_thread->second = false;
                        working_thread_num--;
                        waiting_thread_num++;
                        // cout<<"working thread num:"<<working_thread_num<<endl;
                        // cout<<"waiting wait"<<endl;
                        //等待任务队列不空,或者停止符为真,一旦开始wait,那么就对ul_解锁了
                        // cout<<"cond "<<(reduce_flag || stop_flag || !this->task_q.empty())<<endl;
                        waiting_cv.wait(task_ul, [this](){return (reduce_flag || stop_flag || !this->task_q.empty());});
                        if(stop_flag||reduce_flag) return;//waiting线程出口
                        //正常情况就接任务,并且把自己挂到working队列里
                        working_thread_num++;
                        waiting_thread_num--;
                    }
                }
                task = move(task_q.front());
                task_q.pop();
                this_thread->second = true;
                
            }//这里会析构ul_,释放锁
            //领取任务开始执行
            // cout<<"id:"<<this_thread::get_id()<<endl;
            task();
            // cout<<"task()"<<endl;
            //执行完毕回到开始阻塞
        }
    }

    //submit函数
    template<typename F,typename ...Args>
    auto Submit(F &&f, Args&& ...args)->future<decltype(f(args...))>{
//        cout<<"submit"<<endl;
        using return_type = decltype(f(args...));

        //核心步骤,return_type ()是函数类型 返回值(参数列表)
        function<return_type ()>func = bind(std::forward<F>(f), forward<Args>(args)...);

        //用智能指针创建packaged_task对象pk,用于获得绑定的future
        auto task_ptr = make_shared<packaged_task<return_type()>>(func);

        //包装一个没有返回值,没有参数列表的通用函数,因为单纯的lambda没法入队
        //变成function对象方便入队
        function<void()>wrap_func = [task_ptr](){
            (*task_ptr)();
        };
        //任务队列中任务+1
        {
            unique_lock<mutex>task_ul(task_mutex);//同时只会有一个submit,任务锁
            if(task_q.size()>=max_task){
                //if 队列满,那么扩容
                int increase_num_temp = working_thread_num;
                increase_num = increase_num_temp;
                submit_cv.wait(task_ul,[this](){return this->task_q.size()<=this->max_task;});
                // cout<<"============expand============"<<endl;
            }
            task_q.push(wrap_func);
        }
        //notify
        working_cv.notify_one();//优先通知正在挂起态的working线程
        waiting_cv.notify_one();

        return task_ptr->get_future();
    }
};
