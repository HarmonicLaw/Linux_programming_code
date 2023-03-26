#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>          //memset内存处理函数
#include <stdint.h>         //unit16_t类型
#include <signal.h>
#include <stdlib.h>         //atoi函数
#include <errno.h>

#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 5


// 创建一个类给thread_run传输
class Info{
public:
    std::string _IP;
    uint16_t _port;
    int _sock;

    Info(uint16_t port, std::string ip, int sock)
    {
        _port = port;
        _IP = ip;
        _sock = sock;
    }
};

class TcpServer{
private:
    int _listen_sock;
    struct sockaddr_in _server_addr;

public:
    // 服务代码：接收客户端发送的数据，并打印IP和Port以及数据
    // 并告诉客户端服务端已经接收到数据
    // 为了防止给子线程传递this指针，将其改为stastic
    // 并且由于子线程函数只能接受一个参数，所以我们将Service函数的参数分装为一个类来进行传输
    static void Service(std::string ip, int port, int sock)
    {
        while (1)
        {
            char recv_buf[256];
            ssize_t size = recv(sock, recv_buf, sizeof(recv_buf), 0);
            if (size > 0)
            {
                // 正常读取size字节的数据
                recv_buf[size] = 0;
                std::cout << "[" << ip << "]:[" << port  << "]# "<< recv_buf << std::endl;
                std::string msg = "server get!-> ";
                msg += recv_buf;
                send(sock, msg.c_str(), msg.size(), 0);
            }
            else if (size == 0)
            {
                // 对端关闭
                std::cout << "[" << ip << "]:[" << port  << "]# close" << std::endl;
                break;
            }
            else
            {
            // 出错
            std::cerr << sock << "read error" << std::endl; 
            break;
            }
        }
        close(sock);
        std::cout << "service done" << std::endl;
        // exit(0);  该操作会直接终止线程
    }

    // 创建子线程函数，在该函数内部调用Service服务函数，提供服务
    // 给子线程函数传入一个类
    // 为了不让thread_run多一个this指针这个参数，所以用static修饰该函数
    static void* thread_run(void* arg)
    {
        // 线程分离
        pthread_detach(pthread_self());
        Info info = *(Info*)arg;
        delete (Info*)arg;

        Service(info._IP, info._port, info._sock);
        return NULL;
    }

    // 循环获取连接
    void loop()
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        while(1)
        {
            // 获取连接
            int new_sock = accept(_listen_sock, (struct sockaddr*)&peer, &len);
            if(new_sock < 0)
            {
                std::cout<<"Fail to accept"<<std::endl;
            }

            // 创建子线程
            pthread_t tid;
            uint16_t peer_Port = ntohs(peer.sin_port);
            std::string peer_IP = inet_ntoa(peer.sin_addr);

            Info* info = new Info(peer_Port, peer_IP, new_sock); // 要在子线程中释放这块空间
            pthread_create(&tid, NULL, thread_run, (void*)info);
        }
    }

    TcpServer(uint16_t port = DEFAULT_PORT)
    :_listen_sock(-1)
    {   
        // 初始化服务器地址信息
        _server_addr.sin_family = AF_INET;
        _server_addr.sin_port = htons(port);
        _server_addr.sin_addr.s_addr = INADDR_ANY;
    }

    ~TcpServer()
    {
        if(_listen_sock != -1)
        {
            close(_listen_sock);
            _listen_sock = -1;
        }
        // delete[] _child_pids; //使用new分配数组内存时要使用delete[]来释放
    }



    bool TcpServerInit()
    {
        // 1.创建监听套接字
        _listen_sock = socket(AF_INET,SOCK_STREAM,0);
        if (_listen_sock < 0) {
            std::cerr << "Failed to create listen socket." << std::endl;
            return 1;
        }

        // 2.绑定端口号
        if(bind(_listen_sock,(struct sockaddr*)&_server_addr,(socklen_t)sizeof(_server_addr)) < 0)
        {
            std::cerr << "Failed to bind address and port." << std::endl;
            std::cerr <<"errno"<<errno<<std::endl;
            return 2;
        }

        // 3.将套接字设置为监听套接字
        if(listen(_listen_sock, DEFAULT_BACKLOG) < 0)
        {
            std::cerr << "Failed to start listening." << std::endl;
            return 3;
        }

        // 4.循环接收连接并创建子进程进行业务处理
        loop();
    }
};


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
