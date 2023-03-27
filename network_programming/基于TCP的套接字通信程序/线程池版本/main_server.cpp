#include <iostream>
#include "TcpServer.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }       

    TcpServer* TS = new TcpServer(atoi(argv[1]));
    TS->TcpServerInit();
    
    delete TS;
    
    return 0;
}
