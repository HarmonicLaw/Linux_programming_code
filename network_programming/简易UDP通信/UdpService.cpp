#include <iostream>       //输入输出
#include <stdlib.h>
#include <sys/types.h>    //socket
#include <sys/socket.h>   //socket
#include <arpa/inet.h>    //IP地址转换
#include <netinet/in.h>   //IP地址转换
#include <string>         //字符串
#include <cerrno>         //错误输出
#include <cstdio>
#include <cstring>        //字符串函数

#define NUM 1024

std::string Usage(std::string proc)
{
    std::cout<<"Usage "<<proc<<" port"<<std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
    }
  
    uint16_t UdpPort = atoi(argv[1]);

    // 1 创建套接字
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd < 0)
    {
        std::cerr<<"Fail to create socket"<<" errno ="<<errno<<std::endl;
        return 1;
    }

    // 2. 绑定地址
    struct sockaddr_in UdpServer_addr;
    UdpServer_addr.sin_family = AF_INET;
    UdpServer_addr.sin_port = htons(UdpPort);
    UdpServer_addr.sin_addr.s_addr = INADDR_ANY; //任意地址

    if(bind(sockfd,(struct sockaddr*)&UdpServer_addr,sizeof(UdpServer_addr)) < 0)
    {
        std::cerr<<"Fail to bind socket"<<" errno ="<<errno<<std::endl;
        return 2;
    }

    // 3 提供服务
    // 将客户端输入的命令在服务端执行一遍并返回结果给客户端
    char recv_buf[NUM]; //接收缓冲区
    while(true)
    {
        // 接收消息
        struct sockaddr_in UdpClient_addr;
        socklen_t len = sizeof(UdpClient_addr); //输入输出型参数
        ssize_t ret = recvfrom(sockfd,recv_buf,sizeof(recv_buf)-1,0,(struct sockaddr*)&UdpClient_addr,&len);
        if(ret < 0)
        {
            std::cerr<<"Fail to recvfrom"<<std::endl;
        }

        if(strcmp("exit",recv_buf)==0)
        {
            // 客户端退出时服务端不能执行退出命令
            continue;
        }

        // 创建子进程执行命令，并将执行结果写入文件，然后读取结果，响应给客户端
        if(ret > 0)
        {
            //将获取的消息作为字符串处理
            recv_buf[ret] = '\0';
            FILE* fp = popen(recv_buf,"r");

            std::string str;
            char line[NUM] = {0};
            // 按行读取数据
            while(fgets(line,sizeof(line),fp) != NULL)
            {
                str += line;
            }
             pclose(fp);
             std::cout << "client# " << recv_buf<< std::endl;
             sendto(sockfd,str.c_str(),str.size(),0,(struct sockaddr *)&UdpClient_addr,len);
        }
    }
    return 0;
}

