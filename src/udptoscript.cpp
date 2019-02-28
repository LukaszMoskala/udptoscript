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
#include "config.hpp"


#include <SFML/Network.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <signal.h>
config_t config;

//TODO:
// - configuration file
//   - change port                                                 | DONE
//   - change stop command or disable it                           | DONE
//   - IP-based restriction                                        | PARTIALLY DONE
//     - global (in config file)                                   | DONE
//     - per script (comments in scripts)
//   - script directory location                                   | DONE
//   - rules for sending script output back                        | DONE
//     - Never/Always/On success/On failure/Send only return code  | PARTIALLY DONE, CANCELLED
// - MAKE CODE MORE READABLE!!!!!

//from https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
bool file_existence_tester (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

//from https://stackoverflow.com/questions/125828/capturing-stdout-from-a-system-command-optimally
void execToBuf(string command, char* buf, int buflen) {
  FILE* file=popen(command.c_str(),"r");
  if(!file) {
    return;
  }
  //fgets(buf,buflen,file);
  //fread can read more than 1 line
  fread(buf,buflen,1,file);
  pclose(file);
}
sf::UdpSocket socket;

//from https://stackoverflow.com/questions/6168636/how-to-trigger-sigusr1-and-sigusr2
void my_signal_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        cout<<"received SIGUSR1, reloading config"<<endl;
        destroyconfig(config);
        if(loadconfig(config)) {
          socket.unbind();
          exit(1);
        }
        if(verifyconfig(config)) {
          socket.unbind();
          exit(1);
        }
    }
}
int main() {

  if(loadconfig(config)) return 1;
  if(verifyconfig(config)) return 1;

  signal(SIGUSR1, my_signal_handler);

  socket.bind(config.port);

  char buffer[1024];
  std::size_t received = 0;
  sf::IpAddress sender;
  unsigned short port;
  string s="";

  while( 1 ) {
    socket.receive(buffer, sizeof(buffer), received, sender, port);
    cout<<"Received "<<received<<" from "<<sender.toString()<<":"<<port<<endl;
    bool allowed=false;
    for(int i=0;i<config.globalAllowedIPS.size() && !allowed;i++) {
      allowed= ( config.globalAllowedIPS[i] == sender.toString() );
    }
    if(!allowed) {
      cout<<"IP "<<sender.toString()<<" not in allowed list"<<endl;
      continue;
    }
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
    if(s == config.stopcommand) {
      cout<<"Received stop command, exiting now"<<endl;
      break;
    }

    s=config.scriptsDir+"/"+s;
    cout<<s<<endl;
    if(file_existence_tester(s)) {
      memset(buffer,0,sizeof(buffer));
      execToBuf(s,buffer,sizeof(buffer)-1);
      if(config.OutputSendRules == always)
        socket.send(buffer,strlen(buffer),sender,port);
    }
    else {
      cout<<"Error: file does not exist!"<<endl;
    }


  }
  socket.unbind();
}
