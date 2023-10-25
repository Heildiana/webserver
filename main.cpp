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
#include <list>

using namespace std;

class ThreadPool{
private:
    //queue装function对象,没有返回值的任务队列
    queue<function<void()>>task_q;
    //线程数组
    list<thread*>working_thread;
    list<thread*>waiting_thread;
    //需要两个线程数组 working_thread 和 waiting_thread

    //线程数量
    int thread_num;
    //停止flag
    bool stop_flag;
    bool reduce_flag;//缩容flag
    //条件变量和锁
    mutex mutex_task;
    mutex waiting_mutex;
    mutex working_mutex;

    condition_variable cv_task;
    condition_variable waiting_cv;

    //需要限制任务池的最大容量吗?
    int max_task;


public:
    //构造线程池,初始化线程数组
    ThreadPool(int thread_num): thread_num(thread_num),stop_flag(false),reduce_flag(false){

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
            working_thread.emplace_back(new_thread);
        }
    }

    ~ThreadPool(){//join 所有的线程
        stop_flag= true;//为什么这个flag也需要锁?
        cv_task.notify_all();

        for(auto ite:working_thread){
            ite->join();
        }
        for(auto ite:waiting_thread){
            ite->join();
        }
    }
    //daemon线程动作
    void daemon_thread_work(){
        while (!stop_flag){
            //循环探查waiting队列的每个线程,试图回收 ,if reduce为真的话

            //什么时候扩容线程? 发现队列为满时

            //什么时候缩容线程? waiting队列太多时
            if(waiting_thread.size() > working_thread.size()){
                reduce_flag= true;
                waiting_cv.notify_all();
                //开始join
                for(auto ite:waiting_thread){
                    ite->join();
                }
            } else if(task_q.size()==max_task){
                //扩容
            }
        }
    }

    //线程动作
    //最开始在working队列里
    //试图去task_queue接任务
    //发现queue size为0
    //把自己挂进阻塞队列里面
    void thread_work(future<thread*>& fu){
        thread* my_ptr = fu.get();
//        cout<<my_ptr<<endl;
        sleep(2);
        while (1){
            function<void()>task;
            //阻塞,等待queue中的任务,临界资源是队列,访问任务队列的代码段需要加锁
            {
                unique_lock<mutex>ul_task(mutex_task);//对任务队列上锁
                if(task_q.size()==0){//没有任务那么把自己从working挂到waiting里
                    unique_lock<mutex>wk_ul(working_mutex);
                    unique_lock<mutex>wt_ul(waiting_mutex);
                    working_thread.erase(std::find(working_thread.begin(), working_thread.end(),my_ptr));
                    waiting_thread.push_back(my_ptr);
                    //等待任务队列不空,或者停止符为真,一旦开始wait,那么就对ul_解锁了
                    cout<<"wait"<<endl;
                    waiting_cv.wait(ul_task, [this](){return reduce_flag||stop_flag || !this->task_q.empty();});
                    //如果停止符为真且队列也完了,那么就return;
                    if(stop_flag&&task_q.empty()){
                        return;
                    }
                    if(reduce_flag){
                        return;
                    }
                    //如果缩容flag亮了那么就return
                    //那么缩容flag什么时候恢复呢?当缩容计数器=0的时候就恢复,在daemon那边恢复的

                    //正常情况就接任务,并且把自己挂到working队列里
                    {
                        unique_lock<mutex>wk_ul(working_mutex);
                        unique_lock<mutex>wt_ul(waiting_mutex);
                        waiting_thread.erase(std::find(working_thread.begin(), working_thread.end(),my_ptr));
                        working_thread.push_back(my_ptr);
                    }
                }//这里丢掉两个队列的锁

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
        cout<<"submit"<<endl;
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
        //if 队列满,那么扩容
        {
            unique_lock<mutex>ul(mutex_task);
            task_q.push(wrap_func);
        }
        //notify
        waiting_cv.notify_one();
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
//    ptrClass a;
//    a.printBuffer();
    //将a对象和print成员函数绑定

//    tp.Submit(bind(&ptrClass::printBuffer,&a));
//    tp.Submit(&ptrClass::printBuffer,&a);
    //为什么不能直接把成员函数给进去呢?

    for(int i = 0;i<20;i++){
        tp.Submit(multiply_no_return,i,1);
    }
    sleep(2);
    for(int i = 0;i<20;i++){
        tp.Submit(multiply_no_return,i,1);
    }
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
