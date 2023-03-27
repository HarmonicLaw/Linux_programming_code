#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>  

struct Task
{
  int _port;
  std::string _ip;
	int _sock;
	
	Task(int port, std::string ip, uint16_t sock)
	:_port(port)
	,_ip(ip)
	,_sock(sock)
	{}

    static void Service(std::string ip, int port, uint16_t sock)
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
    }

	void Run()
	{
	    Service(_ip, _port, _sock);
	}
};
