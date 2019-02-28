/*
  WARNING: THIS LIBRARY WORKS ONLY FOR THIS PROGRAM!
  it has next to nothing functionality, doesn't support multiple clients
  and many more limitations!

  packets can be sent only as responses to client that already sent packet
*/
#ifndef NETWORK_HPP
#define NETWORK_HPP 1
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;
class UdpServer {
private:
  int sockfd;
  struct sockaddr_in serv,client;
  socklen_t l;
public:
  //Create socket and bind
  int begin(uint16_t port, string listenip);
  //Read from socket
  int read(char *buf, int buflen);
  //respond to last client
  int respond(char* buf, int buflen);
  //same but with string, used for sending error messages
  void respond(const char* s);
  //get last client IP
  const char* getClientIP();
  //unbind socket
  void cleanup();
};


#endif
