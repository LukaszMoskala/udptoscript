#ifndef NETWORK_HPP
#define NETWORK_HPP 1

class UdpServer {
private:
  int socket;
  struct sockaddr_in serv,client;
public:
  begin(int port);
  int read(char *buf, int buflen);
  int respond(char* buf, int buflen);
  void cleanup();
};


#endif
