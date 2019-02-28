#ifndef NETWORK_HPP
#define NETWORK_HPP 1
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<iostream>
#include<cstdio>
#include<cstring>
using namespace std;
class UdpServer {
private:
  int sockfd;
  struct sockaddr_in serv,client;
  socklen_t l;
public:
  int begin(uint16_t port);
  int read(char *buf, int buflen);
  int respond(char* buf, int buflen);
  const char* getClientIP();
  void cleanup();
};


#endif
