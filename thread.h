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

using namespace std;

class ThreadPool{
private:
    //queue装function对象,没有返回值的任务队列
    queue<function<void()>>task_q;
    //线程数组
    list<thread*>working_threads;
    list<thread*>waiting_threads;
    thread* daemon;
    //需要两个线程数组 working_threads 和 waiting_threads

    //线程数量
    int thread_num;
    //停止flag
    bool stop_flag;
    bool reduce_flag;//缩容flag
    //条件变量和锁
    mutex task_mutex;
    mutex threads_mutex;//一个锁锁两个队列
//    mutex working_mutex;

    condition_variable task_cv;
    condition_variable waiting_cv;

    //需要限制任务池的最大容量吗?
    int max_task;


public:
    //构造线程池,初始化线程数组
    ThreadPool(int thread_num): thread_num(thread_num),stop_flag(false),reduce_flag(false),max_task(thread_num*2){

        //需要给出一个常驻回收线程 1 回收 2 打印当前线程数量 daemon线程

        for(int i = 0; i < thread_num; i++){
            promise<thread*>pro;
            future<thread*>fu = pro.get_future();
            //future 应该是不能被拷贝构造,只能传引用
            //future 怎么传到lambda表达式里面
            thread* new_thread = new thread(bind(
                    [this](future<thread*>&fu){this->thread_work(fu);},move(fu)
                    )
                            );
            pro.set_value(new_thread);
            working_threads.emplace_back(new_thread);
        }
        daemon = new thread([this](){ this->daemon_thread_work();});
    }

    ~ThreadPool(){//join 所有的线程
        {
            unique_lock<mutex>task_ul(task_mutex);
            stop_flag= true;//为什么这个flag也需要锁?
//            cout<<"stop:"<<stop_flag<<endl;
        }
        //设置为true后,当task队列空时,线程返回
        daemon->join();
    }

    //daemon线程动作
    void daemon_thread_work(){
        while (!stop_flag){
            //什么时候扩容线程? 发现队列为满时=====这个交给submit函数做了

            //什么时候缩容线程? waiting队列太多时
            sleep(2);
            {
                unique_lock<mutex>thread_ul(threads_mutex);
                cout<<"==============thread num: "<<waiting_threads.size()+working_threads.size()<<endl;
                if(waiting_threads.size() > working_threads.size()){
                    reduce_flag= true;
                    waiting_cv.notify_all();
                    //开始join
                    for(auto ite:waiting_threads){
                        ite->join();
                        waiting_threads.erase(std::find(waiting_threads.begin(), waiting_threads.end(),ite));
                        delete ite;
                    }
                } //如果没有很多空闲进程,那么马上丢锁
            }
        }
        //stop flag 为真 需要join所有线程
        {
            unique_lock<mutex>wt_ul(threads_mutex);//对两个队列加锁
            waiting_cv.notify_all();

            //程序依赖本行注释运行,没有这行注释会偶尔死锁

            for(auto ite:working_threads){
                ite->join();
                delete ite;
            }
            cout<<"joing working"<<endl;

            for(auto ite:waiting_threads){
                ite->join();
                delete ite;
            }
            cout<<"joing waiting"<<endl;
        }
        return;
    }

    //线程动作
    //最开始在working队列里
    //试图去task_queue接任务
    //发现queue size为0
    //把自己挂进阻塞队列里面
    void thread_work(future<thread*>& fu){
        thread* my_ptr = fu.get();
//        cout<<my_ptr<<endl;
//        sleep(2);
        while (1){
            function<void()>task;
            //阻塞,等待queue中的任务,临界资源是队列,访问任务队列的代码段需要加锁
            {
                unique_lock<mutex>task_ul(task_mutex);//对任务队列上锁
//                cout<<"th stop:"<<stop_flag<<endl;
//                cout<<"task_q size:"<<task_q.size()<<endl;
                if(task_q.size()==0){//没有任务那么把自己从working挂到waiting里
//                    cout<<"working to waiting"<<endl;
                    if(stop_flag) return;//working线程出口
                    //如果缩容flag亮了那么就return
                    //那么缩容flag什么时候恢复呢?当缩容计数器=0的时候就恢复,在daemon那边恢复的

                    {//这里容易死锁,当析构函数析构拿锁时,其他线程会卡着
                        unique_lock<mutex>wt_ul(threads_mutex);
                        working_threads.erase(std::find(working_threads.begin(), working_threads.end(), my_ptr));
                        waiting_threads.push_back(my_ptr);
                    }
                    //等待任务队列不空,或者停止符为真,一旦开始wait,那么就对ul_解锁了
//                    cout<<"wait"<<endl;
                    waiting_cv.wait(task_ul, [this](){return reduce_flag || stop_flag || !this->task_q.empty();});
                    //丢任务锁
                    if(stop_flag||reduce_flag) return;//waiting线程出口

                    //正常情况就接任务,并且把自己挂到working队列里
                    {
                        unique_lock<mutex>wt_ul(threads_mutex);
                        waiting_threads.erase(std::find(waiting_threads.begin(), waiting_threads.end(), my_ptr));
                        working_threads.push_back(my_ptr);
//                        cout<<"waiting to working"<<endl;
                    }//这里丢掉两个队列的锁
                }

                task = move(task_q.front());
                task_q.pop();
            }//这里会析构ul_,释放锁

            //领取任务开始执行
            cout<<"id:"<<this_thread::get_id()<<endl;
            task();
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
            unique_lock<mutex>ul(task_mutex);//同时只会有一个submit
            if(task_q.size()>=max_task){
                //if 队列满,那么扩容
                {//拿线程队列的锁
                    unique_lock<mutex>thread_ul(threads_mutex);
                    for(int i = 0; i < thread_num; i++){
                        promise<thread*>pro;
                        future<thread*>fu = pro.get_future();
                        //future 应该是不能被拷贝构造,只能传引用
                        //future 怎么传到lambda表达式里面
                        thread* new_thread = new thread(bind(
                                                                [this](future<thread*>&fu){this->thread_work(fu);},move(fu)
                                                        )
                        );
                        pro.set_value(new_thread);
                        working_threads.emplace_back(new_thread);
                    }
                }
                max_task*=2;
                thread_num*=2;
                cout<<"============expand============"<<endl;
            }
            task_q.push(wrap_func);
        }
        //notify
        waiting_cv.notify_one();

        return task_ptr->get_future();
    }
};
