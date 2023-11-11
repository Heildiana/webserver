
#if 1
#include "webserver.h"

int main(){
    Webserver my_server(1316,10);
    ofstream fout;
    fout.open("server.txt",ios::out);
    if (!fout.is_open()) {
        std::cerr << "Error opening file 'thread_out.txt'" << std::endl;
        return 1; // 退出程序，返回错误码
    }
    if (!fout.good()) {
        std::cerr << "Error with file stream." << std::endl;
        return 1; // 退出程序，返回错误码
    }
    cout.rdbuf(fout.rdbuf());
    my_server.mainFrame();
    return 0;
}
#endif