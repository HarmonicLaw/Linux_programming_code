#pragma once

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

#include "thread_pool.hpp"
#include "Task.hpp"

#define DEFAULT_PORT 8080
#define DEFAULT_BACKLOG 5

using namespace ns_threadpool;

class TcpServer{
private:
    int _listen_sock;
    struct sockaddr_in _server_addr;

public:
    // 循环获取连接
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
            
            int peer_Port = ntohs(peer.sin_port);
            std::string peer_IP = inet_ntoa(peer.sin_addr);
            std::cout << "get a new link, [" << peer_IP << "]:[" << peer_Port  << "]"<< std::endl;
            
            Task task(peer_Port, peer_IP, sock);
            ThreadPool<Task>::GetInstance()->PushTask(task);
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
