#include <vector>
#include <iostream>


class myClassA{
private:
    int a_;
public:
    // myClassA(){};
    myClassA(int a):a_(a){
        std::cout<<"myClass ctor"<<std::endl;
    }

};

class myClassB{
private:
    myClassA m1_;
public:
    myClassB(int a):m1_(a){
        std::cout<<"myClass1 ctor"<<std::endl;
    }

    // myClassB(myClassA m1){
    //     m1_ = m1;
    //     std::cout<<"myClass1 move ctor"<<std::endl;
    // }

};

int main(){
    myClassA a();
    // myClassB my(1);
    
    // myClassB my1(myClassA());
    return 0;
}