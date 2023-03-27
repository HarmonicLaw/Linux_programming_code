#include <iostream>
#include "TcpClient.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout<<"Usage"<<argv[0]<<" IP "<<" port "<<std::endl;
        return 1;
    }   

    TcpClient* TC = new TcpClient(argv[1], atoi(argv[2]));

    TC->TcpClientInit();
    TC->TcpClientConnect();
    TC->Request();

    delete TC;
    return 0;
}
