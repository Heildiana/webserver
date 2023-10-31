#include <iostream>
#include <unordered_map>


class classA
{
private:
    /* data */
public:
    classA(/* args */);
    classA(int );
    ~classA();
    void func();
};

classA::classA(/* args */)
{
    std::cout<<"A ctor"<<std::endl;
}

classA::classA(int)
{
    std::cout<<"int ctor"<<std::endl;
}

classA::~classA()
{
}

void classA::func()
{
    std::cout<<"func"<<std::endl;
}

int main(){
    std::unordered_map<int,classA>map;
    map[1].func();
}