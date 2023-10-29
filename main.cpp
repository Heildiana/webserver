////
////  main.cpp
////  ThreadPool
////
////  Created by Yiping Wang on 2023/10/25.
////
//
//#include <iostream>
//#include "ThreadPool.hpp"
//#include "ThreadPool2.hpp"
//#include <mutex>
//#include <vector>
//#include <atomic>
//
//std::mutex m;
//
//std::atomic<int> num = 0;
//
//class A {
//public:
//    static void print(){
////        std::cout << "Hello" << std::endl;
//    }
//    void pp() {
////        std::cout << "pp" << std::endl;
//    }
//    explicit A(int v) {
////        std::cout << "A()" << std::endl;
//        this->x = new int;
//        *(this->x) = v;
//    }
//    A(const A &a) {
//        num ++;
//        std::cout << "+";
////        std::cout << "A(A&)" << std::endl;
//        this->x = new int;
//        *(this->x) = *(a.x);
//    }
//    A(A &&a) noexcept {
////        std::cout << "A(A&&)" << std::endl;
//        this->x = a.x;
//        a.x = nullptr;
//    }
//    ~A() {
////        std::cout << "~A()" << std::endl;
//        delete this->x;
//    }
//    int *x;
//};
///*
//void mainx() {
//    std::cout << "Hello, Let's begin" << std::endl;
//    ThreadPool tp(10);
//    std::vector<std::future<A>> res;
//    for(int i = 0; i < 20; i ++) {
//        res.push_back(tp.submit([] (int i, A a)-> A {
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            {
//                std::unique_lock<std::mutex> lk(m);
//                std::cout << "I am " << i << std::endl;
//            }
//            *(a.x) = i;
//            return a;
//        }, i * i, A(10)));
//    }
//    tp.addThreads(10);
//    for(int i = 20; i < 40; i ++) {
//        res.push_back(tp.submit([] (int i, A a)-> A {
//            std::this_thread::sleep_for(std::chrono::seconds(3));
//            {
//                std::unique_lock<std::mutex> lk(m);
//                std::cout << "I am " << i << std::endl;
//            }
//            *(a.x) = i;
//            return a;
//        }, i * i, A(31)));
//    }
//    std::cout << "all over" << std::endl;
//    for(auto & i : res) {
//        std::cout << *((i.get()).x) << std::endl;
//    }
//}
//
//*/
//
//void mainy() {
//    ThreadPool2 tp(30);
//    std::cout << "begin" << std::endl;
//    for(int i = 0; i < 3000; i ++) tp.submit([i](A a) {
////        std::cout << "+";
////        std::this_thread::sleep_for(std::chrono::seconds(1));
////        std::cout << "-";
//        double x = 1;
//        for(int i = 0; i < 1000000; i ++) {
//            if(x > 3) x /= 2;
//            else x *=1.024;
//        }
//    }, A(10));
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    tp.submit([](A a) {
////        std::cout << "+";
//        std::this_thread::sleep_for(std::chrono::seconds(1));
////        std::cout << "-";
//    }, A(12));
//    std::this_thread::sleep_for(std::chrono::seconds(10));
//    for(int i = 0; i < 3000; i ++) tp.submit([i](A a) {
////        std::cout << "+";
//        double x = 1;
//        for(int i = 0; i < 1000000; i ++) {
//            if(x > 3) x /= 2;
//            else x *=1.024;
//        }
////        std::cout << "-";
//    }, A(10));
//    std::cout << num << std::endl;
//}
//
//int main() {
//    mainy();
//}
//
//////
//////
////// Created by zzk on 2023/10/26.
//////
////#include "ThreadPool2.hpp"
////using namespace std;
////#include <unistd.h>
////
////class ptrClass{
////private:
////    char* buffer;
////public:
////    ptrClass(){//无参构造
////        buffer = new char [20]{"helllll"};
////        cout<<"ctor"<<endl;
////    }
////    ptrClass(ptrClass& pt){//拷贝构造
////        cout<<"copy ctor"<<endl;
////        strcpy(this->buffer,pt.buffer);
////    }
////    ptrClass(ptrClass&& pt){//移动构造
////        cout<<"move ctor"<<endl;
////        this->buffer = pt.buffer;
////        pt.buffer = nullptr;
////    }
////    void printBuffer(){
////        cout<<buffer<<endl;
////    }
////};
////
////
////class OtherClass{
////public:
////    void multiply(int a,int b){
////        cout<<a*b<<endl;
////    }
////};
////
////
//////简单乘法任务
////void multiply_no_return(int a,int b){
////    double y = 0.15;
////    for(int i = 0; i < 1000000; i ++) {
////        if(y > 10) y /= 2;
////        else y *= 1.05;
////    }
////    a*b;
////}
////
////int multiply_with_return(int a,int b){
////    return a*b;
////}
////
////int main(){
////    cout<<"hello"<<endl;
////
////    ThreadPool2 tp(10);
////
////    ptrClass a;
////    tp.submit(bind(&ptrClass::printBuffer,&a));//将a对象和print成员函数绑定
////
//////    a.printBuffer();
////
//////    tp.Submit(&ptrClass::printBuffer,&a);
////    //为什么不能直接把成员函数给进去呢?
////    tp.submit(multiply_no_return,1,1);
////
////    int times = 100000;
////    cout<<times<<endl;
////    for(int i = 0;i<times;i++){
////        tp.submit(multiply_no_return,i,1);
////    }
////    std::cout << "over" << std::endl;
////
//////    sleep(1);
//////
//////    for(int i = 0;i<10000;i++){
//////        tp.Submit(multiply_no_return,i,1);
//////    }
////}

#include "threadsPool.h"
#include <unistd.h>
#include <iostream>

int main() {
    struct sched_param sched_param;
    sched_param.sched_priority = 90;
    pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
    ThreadsPool tp(40);
    for(int i = 0; i < 1000000; ++ i) {
        tp.submit([](){
            double x = 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(90));
//            for(int i = 0; i < 1000000; i ++) {
//                if(x > 2) x /= 2;
//                else x *= 1.0245;
//                std::this_thread::sleep_for(std::chrono::microseconds(90));
//            }
        });
    }
    std::cout << "over" << std::endl;
//    sleep(20);
}
