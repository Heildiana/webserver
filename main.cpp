//
// Created by zzk on 2023/10/26.
//
#include "thread.h"


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
    tp.Submit(bind(&ptrClass::printBuffer,&a));//将a对象和print成员函数绑定

//    a.printBuffer();

//    tp.Submit(&ptrClass::printBuffer,&a);
    //为什么不能直接把成员函数给进去呢?


    tp.Submit(multiply_no_return,1,1);


    for(int i = 0;i<50;i++){
        tp.Submit(multiply_no_return,i,1);
    }

    sleep(1);

    for(int i = 0;i<50;i++){
        tp.Submit(multiply_no_return,i,1);
    }
    sleep(2);
    cout<<"destory threadpool"<<endl;
}