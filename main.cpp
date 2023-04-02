#include"muduo/base/Logging.h"
#include"PetsServer.hpp"
#include"Define.hpp"


int main(int argc, char* argv[])
{
    net::EventLoop loop;
    uint16_t port = 8080;
    net::InetAddress serverAddr(port);
    PetsServer server(&loop, serverAddr);
    if(!server.start())
        return -1;
    loop.loop();
    return 0;
}