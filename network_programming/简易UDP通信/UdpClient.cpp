#include <iostream>       //输入输出
#include <sys/types.h>    //socket
#include <sys/socket.h>   //socket
#include <arpa/inet.h>    //IP地址转换
#include <netinet/in.h>   //IP地址转换
#include <string>         //字符串
#include <cerrno>         //错误输出
#include <cstring>        //字符串函数
#include <cstdlib>        //atoi函数
#include <cstdio>         //fgets函数 
#include <stdio.h>

std::string Usage(std::string proc)
{
  std::cout<<"Usage "<< proc <<" IP port"<<std::endl;
  return 0;
}

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    Usage(argv[0]);
    return 0;
  }

  // 1. 创建套接字
  int sockfd = socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd < 0)
  {
    std::cerr<<"Fail to create sock"<<std::endl;
    return 1;
  }

  // 2. 绑定地址(自动绑定本地)

  // 3. 填写目标服务器地址，以请求服务
  struct sockaddr_in UdpServer_addr;
  UdpServer_addr.sin_family = AF_INET;
  UdpServer_addr.sin_port = htons(atoi(argv[2]));
  UdpServer_addr.sin_addr.s_addr = inet_addr(argv[1]);

  // 4. 使用服务
  while(true)
  {
    std::cout<<"MyShell $";
    char line[1024];
    fgets(line, sizeof(line), stdin); //自动在末尾添加’\0‘
    if(strcmp("exit",line)==0)
    {
      return 0;
    }

    sendto(sockfd,line,strlen(line),0,(struct sockaddr*)&UdpServer_addr,sizeof(UdpServer_addr));
    
    struct sockaddr_in tmp;
    socklen_t len = sizeof(tmp);
    char recv_buf[1024];
    // 发送完毕后阻塞式接收响应
    ssize_t ret = recvfrom(sockfd,recv_buf,sizeof(recv_buf),0,(struct sockaddr*)&tmp,&len);
    if(ret > 0)
    {
      recv_buf[ret] = 0;
      std::cout << recv_buf << std::endl;
    }
  }
  return 0;
}


