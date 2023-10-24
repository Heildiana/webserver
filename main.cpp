#include <iostream>
#include <functional>
#include <unistd.h>
#include <memory>
#include <vector>
#include <future>
#include <queue>

using namespace std;

class threadPool{
private:


public:
    //queue装function对象
    queue<function<void()>>task_q;
    //submit函数
    template<typename F,typename ...Args>
    auto submit(F &&f, Args&& ...args)->future<decltype(f(args...))>{
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

        task_q.push(wrap_func);

        return task_ptr->get_future();
    }
};

class ptrClass{
private:
    char* buffer;
public:
    ptrClass(){
        buffer = new char [20]{"helllll"};
        cout<<"ctor"<<endl;
    }
    ptrClass(ptrClass& pt){
        cout<<"copy ctor"<<endl;
    }
    ptrClass(ptrClass&& pt){
        cout<<"move ctor"<<endl;
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
    threadPool tp;
    int x(1),y(2);
    future<int>fu = tp.submit([](int &x,int &y){return x*y;},1,2);

    tp.task_q.front()();
    cout<<fu.get()<<endl;


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
