/*
Copyright (C) 2019 Łukasz Konrad Moskała

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//Most of network code is copied from SFML website:
// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1UdpSocket.php

#include <iostream>
#include <SFML/Network.hpp>
#include <sys/stat.h>

using namespace std;

//TODO:
// - configuration file
//   - change port
//   - change stop command or disable it
//   - IP-based restriction
//   - script directory location
//MAKE CODE MORE READABLE!!!!!

//from https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
bool file_existence_tester (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int main() {

  sf::UdpSocket socket;
  socket.bind(4404);

  char buffer[1024];
  std::size_t received = 0;
  sf::IpAddress sender;
  unsigned short port;
  string s="";

  while( s != "stopdaemon" ) {
    socket.receive(buffer, sizeof(buffer), received, sender, port);
    cout<<"Received "<<received<<" from "<<sender.toString()<<":"<<port<<endl;
    bool possiblygood=true;

    for(int i=0;i<received && possiblygood;i++) {
      possiblygood=( ( buffer[i] >= '0' && buffer[i] <= '9' ) ||
                     ( buffer[i] >= 'a' && buffer[i] <= 'z' ) ||
                     ( buffer[i] >= 'A' && buffer[i] <= 'Z' ) ||
                       buffer[i] == '.');
    }

    if(!possiblygood)
      continue;
    cout<<"Data is valid, continue"<<endl;
    s=string(buffer, received);

    s="scripts/"+s;
    cout<<s<<endl;
    if(file_existence_tester(s)) {
      system(s.c_str());
    }


  }

}
