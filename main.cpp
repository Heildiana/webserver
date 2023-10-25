#include <iostream>
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

using namespace std;

class ThreadPool{
private:
    //queue装function对象,没有返回值的任务队列
    queue<function<void()>>task_q;
    //线程数组
    vector<thread>thread_vec;
    //线程数量
    int thread_num;
    //停止flag
    bool stop_flag;
    //条件变量和锁
    mutex mutex_;
    condition_variable cv_;
    //需要限制任务池的最大容量吗?


public:
    //构造线程池,初始化线程数组
    ThreadPool(int thread_num): thread_num(thread_num),stop_flag(false){

        for(int i = 0; i < thread_num; i++){
            thread_vec.emplace_back([this](){ this->thread_work();});
        }
    }

    ~ThreadPool(){//join 所有的线程
        stop_flag= true;//为什么这个flag也需要锁?
        cv_.notify_all();
        for(int i = 0;i<thread_num;i++){
            thread_vec[i].join();
        }
    }


    //线程动作
    void thread_work(){
        while (1){
            function<void()>task;
            //阻塞,等待queue中的任务,临界资源是队列,访问任务队列的代码段需要加锁
            {
                unique_lock<mutex>ul_(mutex_);
                //等待任务队列不空,或者停止符为真
                cv_.wait(ul_,[this](){return stop_flag||!this->task_q.empty();});
                //如果停止符为真且队列也完了,那么就return;
                if(stop_flag&&task_q.empty()){
                    return;
                }
                //正常情况就接任务
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
        task_q.push(wrap_func);
        //notify
        cv_.notify_one();
        return task_ptr->get_future();
    }
};

class ptrClass{
private:
    char* buffer;
public:
    ptrClass(){//无参构造
        buffer = new char [20]{"helllll"};
        cout<<"ctor"<<endl;
    }
    ptrClass(ptrClass& pt){//拷贝构造
        cout<<"copy ctor"<<endl;
        strcpy(this->buffer,pt.buffer);
    }
    ptrClass(ptrClass&& pt){//移动构造
        cout<<"move ctor"<<endl;
        this->buffer = pt.buffer;
        pt.buffer = nullptr;
    }
    void printBuffer(){
        cout<<buffer<<endl;
    }
};


class OtherClass{
public:
    void multiply(int a,int b){
        cout<<a*b<<endl;
    }
};


//简单乘法任务
void multiply_no_return(int a,int b){
    sleep(1);
    cout<<a*b<<endl;
}

int multiply_with_return(int a,int b){
    sleep(1);
    return a*b;
}




int main(){
    ThreadPool tp(10);
    ptrClass a;
//    a.printBuffer();
    //将a对象和print成员函数绑定

    tp.Submit(bind(&ptrClass::printBuffer,&a));
//    tp.Submit(&ptrClass::printBuffer,&a);
    //为什么不能直接把成员函数给进去呢?

    OtherClass o;
//    tp.Submit(&OtherClass::multiply,&o,1,2);
}






#if 0
int main() {//测试
    function<int()>func = bind(multiply_with_return,1,2);//核心步骤
    cout<<func()<<endl;

    //获得future
    auto task_ptr = make_shared<packaged_task<int()>>(func);

    //包装成void()入队
    queue<function<void()>>func_q;//创建队列
    function<void()>wrap_task =  [task_ptr](){
        (*task_ptr)();//task_ptr它自己无法执行,但是它指向的东西可以执行
        //package类内部重载了()运算符,是个可调用对象,调用构造时穿进去的函数
    };
    wrap_task();//执行队列里的函数,实际情况下,这个由另一个线程完成

    cout<<task_ptr->get_future().get()<<endl;//主线程通过future获取返回值
}
#endif
