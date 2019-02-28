/*
  WARNING: THIS LIBRARY WORKS ONLY FOR THIS PROGRAM!
  it has next to nothing functionality, doesn't support multiple clients
  and many more limitations!

  packets can be sent only as responses to client that already sent packet
*/
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
