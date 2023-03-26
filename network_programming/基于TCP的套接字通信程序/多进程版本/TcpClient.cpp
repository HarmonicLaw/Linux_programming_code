#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>          //memset内存处理函数
#include <stdint.h>         //unit16_t类型
#include <cerrno>           //错误输出
#include <string>       
#include <stdlib.h>         //exit函数
#include <cstdio>           //fgets函数
#include <stdio.h>          //stdin


#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PORT 8080

// using namespace std;

class TcpClient{
private:
    int _sock;
    // string _server_ip;
    // uint16_t _server_port;
    struct sockaddr_in _serv_addr;

public:
    TcpClient(std::string server_ip = DEFAULT_SERVER_IP, uint16_t server_port = DEFAULT_SERVER_PORT) 
    //:_server_ip(server_ip), _server_port(server_port)
    :_sock(-1)
    {
        // 设置服务器地址
        memset(&_serv_addr, 0, sizeof(_serv_addr));
        _serv_addr.sin_family = AF_INET;
        _serv_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
        _serv_addr.sin_port = htons(server_port);
    }

    ~TcpClient()
    {
        if (_sock >= 0) 
            close(_sock);
    }

    bool TcpClientInit()
    {
        // 1.创建socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (_sock == -1) {
            std::cerr << "Error: Failed to create socket." <<std:: endl;
            return false;
        }
        return true;
    }

    void TcpClientConnect()
    { 
        // 当一个TCP客户端调用connect()函数时，如果没有绑定本地地址信息，则会自动绑定一个 
        if (connect(_sock, (struct sockaddr*)&_serv_addr, sizeof(_serv_addr)) < 0)
        {
            // 连接失败
            std::cerr << "connect fail" << std::endl;
            exit(-1);
        }
    }


    void Request()
    {
        while (1)
        {
            std::cout << "Client # ";
            char line[1024];
            fgets(line, sizeof(line), stdin); //自动在末尾添加’\0‘

            send(_sock, line, strlen(line), 0);

            char recv_buf[1024];
            ssize_t size = recv(_sock, recv_buf, sizeof(recv_buf), 0);

            if (size <= 0){
                std::cerr << "read error" << std::endl;
                exit(-1);
            }

            recv_buf[size] = 0;
            std::cout << recv_buf << std::endl;
        }
    }

};

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

