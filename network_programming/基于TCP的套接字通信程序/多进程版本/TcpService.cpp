#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>          //memset内存处理函数
#include <stdint.h>         //unit16_t类型
#include <signal.h>
#include <stdlib.h>         //atoi函数
#include <errno.h>
#include <sys/wait.h>       //waitpid函数

#define DEFAULT_PORT 8080
// #define DEFAULT_CHILD_COUNT 10
#define DEFAULT_BACKLOG 5
#define WAY 0

class TcpServer{
private:
    int _listen_sock;
    struct sockaddr_in _server_addr;
    // pid_t* _child_pids;
    // int _max_child_count;

    #if WAY
    // 服务代码：接收客户端发送的数据，并打印IP和Port以及数据
    // 并告诉客户端服务端已经接收到数据
    void Service(std::string ip, int port, int sock)
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
        // 子进程退出
        exit(0);
    }

    // 循环获取连接
    void loop()
    {
        // 对SIGCHLD信号进行注册，处理方式为忽略
        signal(SIGCHLD, SIG_IGN);

        struct sockaddr_in peer;// 获取远端端口号和ip信息
        socklen_t len = sizeof(peer);
        while (1)
        { 
            // 获取链接 
            // sock 是进行通信的一个套接字  _listen_sock 是进行监听获取链接的一个套接字 
            int sock = accept(_listen_sock, (struct sockaddr*)&peer, &len); 
            if (sock < 0)
            { 
                // 一次获取连接失败不要直接将服务端关闭，而是重新去获取连接            
                std::cout << "accept fail, continue accept" << std::endl;
                continue; 
            }
            
            // 创建子进程
            pid_t id = fork();
            if (id == 0)
            {
                // 子进程
                close(_listen_sock); // 可以不关闭，但是建议关闭，以防后期子进程对监听套接字fd进行了一些操作，对父进程造成影响
                uint16_t peer_Port = ntohs(peer.sin_port);
                std::string peer_IP = inet_ntoa(peer.sin_addr);
                std::cout << "get a new link, [" << peer_IP << "]:[" << peer_Port  << "]"<< std::endl;

                // 子进程想客户端(peer)提供服务
                Service(peer_IP, peer_Port, sock);
                // 服务完成后关闭套接字文件，并退出，在Service中完成
            }
            // 父进程继续去获取连接
        }
    }


    #else
    
    
    void Service(std::string ip, int port, int sock)
    {
        while (1)
        {
            char recv_buf[256];
            ssize_t size = recv(sock, recv_buf, sizeof(recv_buf),0);
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
    // 孙子进程退出
    exit(0);
    }

    void loop()
    {
        struct sockaddr_in peer;// 获取远端端口号和ip信息
        socklen_t len = sizeof(peer);
        while (1)
        { 
            // 获取链接 
            // sock 是进行通信的一个套接字  _listen_sock 是进行监听获取链接的一个套接字 
            int sock = accept(_listen_sock, (struct sockaddr*)&peer, &len); 
            if (sock < 0)
            { 
                std::cout << "accept fail, continue accept" << std::endl; 
                continue; 
            } 
            // 创建子进程
            pid_t id = fork();
            if (id == 0)
            {
                // 子进程
                // 父子进程的文件描述符内容一致
                // 子进程可以关闭监听套接字的文件描述符
                close(_listen_sock); // 可以不关闭，但是建议关闭，以防后期子进程对监听套接字fd进行了一些操作，对父进程造成影响
                if (fork() > 0)
                {
                    // 子进程直接退出，让孙子进程被OS（1号进程）领养，退出时资源被操作系统回收
                    exit(0);
                }
                // 孙子进程
                uint16_t peer_Port = ntohs(peer.sin_port);
                std::string peer_IP = inet_ntoa(peer.sin_addr);
                std::cout << "get a new link, [" << peer_IP << "]:[" << peer_Port  << "]"<< std::endl;
                Service(peer_IP, peer_Port, sock);
            }
        
            // 关闭sock  如果不关闭，那么爷爷进程可用文件描述符会越来越少
            close(sock);
            // 爷爷进程等儿子进程
            waitpid(-1, NULL, 0);// 以阻塞方式等待，但这里不会阻塞，因为儿子进程是立即退出的
        }
    }

    #endif

public:
    TcpServer(uint16_t port = DEFAULT_PORT)
    :_listen_sock(-1)
    // ,_max_child_count = max_child_count
    {   
        // 初始化服务器地址信息
        _server_addr.sin_family = AF_INET;
        _server_addr.sin_port = htons(port);
        _server_addr.sin_addr.s_addr = INADDR_ANY;
        
        //_child_pids = new pid_t[_max_child_count];
        //memset(_child_pids,0,sizeof(_child_pids));
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
