#include <iostream>
#include <regex>
using namespace std;

int main(){

    string target = "POST /pornhub.com HTTP/1.17/r/n";
    smatch subStr;
    regex myPattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)/r/n$");
    regex_match(target,subStr,myPattern);
    cout<<subStr.size()<<endl;
    cout<<subStr[0]<<endl;
    cout<<subStr[1]<<endl;
    cout<<subStr[2]<<endl;
    cout<<subStr[3]<<endl;
    return 0;
}