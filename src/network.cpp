#include "network.hpp"
//lot of code is from http://www.cplusplus.com/forum/unices/76180/
int UdpServer::begin(uint16_t port) {
  this->sockfd=socket(AF_INET, SOCK_DGRAM, 0);
  if(this->sockfd == -1) {
    cout<<"Failed to create socket: "<<strerror(errno)<<endl;
    return 1;
  }
  this->serv.sin_family = AF_INET;
  this->serv.sin_port = htons(port);
//  this->serv.sin_addr.s_addr = inet_addr("127.0.0.1");
  this->serv.sin_addr.s_addr = INADDR_ANY;
  this->l=sizeof(this->client);

  if(bind(this->sockfd,(const struct sockaddr*)&this->serv, this->l)) {
    cout<<"Failed to bind: "<<strerror(errno)<<endl;
  }

  return 0;
}
int UdpServer::read(char* buf, int buflen) {
  int rc=recvfrom(this->sockfd, buf, buflen, 0, (struct sockaddr *)&this->client,&this->l);
  if(rc < 0) {
    cout<<"recvfrom() failed: "<<strerror(errno)<<endl;
  }
  return rc;
}
int UdpServer::respond(char* buf, int buflen) {
  int rp=sendto(this->sockfd, buf, buflen, 0, (struct sockaddr *)&this->client,this->l);
  if(rp < 0) {
    cout<<"sendto() failed: "<<strerror(errno)<<endl;
  }
  return rp;
}
void UdpServer::cleanup() {
  close(this->sockfd);
}
const char* UdpServer::getClientIP() {
  return inet_ntoa(this->client.sin_addr);
}
