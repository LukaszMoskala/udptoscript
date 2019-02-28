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
#include "config.hpp"
#include "network.hpp"
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
UdpServer udp;

//from https://stackoverflow.com/questions/6168636/how-to-trigger-sigusr1-and-sigusr2
void my_signal_handler(int signum)
{
  if (signum == SIGUSR1)
    {
    cout<<"received SIGUSR1, reloading config"<<endl;
    destroyconfig(config);
    if(loadconfig(config)) {
      //exit if we failed to load config
      udp.cleanup();
      exit(1);
    }
    if(verifyconfig(config)) {
      //config is invalid, exit
      udp.cleanup();
      exit(1);
    }
  }
  if(signum == SIGTERM) {
    cout<<"received SIGTERM, exiting gracefully"<<endl;
    udp.cleanup();
    exit(0);
  }
  //^C sends SIGINT
  if(signum == SIGINT) {
    cout<<"received SIGINT, exiting gracefully"<<endl;
    udp.cleanup();
    exit(0);
  }
}
int main() {

  if(loadconfig(config)) return 1;
  if(verifyconfig(config)) return 1;

  signal(SIGUSR1, my_signal_handler);
  signal(SIGTERM, my_signal_handler);
  signal(SIGINT, my_signal_handler);
  if(udp.begin(config.port))
    return 1;
  char buffer[1024];
  int received = 0;
  string s="";

  while( 1 ) {
    //socket.receive(buffer, sizeof(buffer), received, sender, port);
    //cout<<"Received "<<received<<" from "<<sender.toString()<<":"<<port<<endl;
    received=udp.read(buffer, sizeof(buffer));
    if(received < 1)
      continue;
    bool allowed=false;
    for(int i=0;i<config.globalAllowedIPS.size() && !allowed;i++) {
      allowed= ( config.globalAllowedIPS[i] == udp.getClientIP() );
    }
    if(!allowed) {
      cout<<"IP "<<udp.getClientIP()<<" not in allowed list"<<endl;
      continue;
    }
    bool possiblygood=true;

    for(int i=0;i<received && possiblygood;i++) {
      possiblygood=( ( buffer[i] >= '0' && buffer[i] <= '9' ) ||
                     ( buffer[i] >= 'a' && buffer[i] <= 'z' ) ||
                     ( buffer[i] >= 'A' && buffer[i] <= 'Z' ) ||
                       buffer[i] == '.');
    }

    if(!possiblygood) {
      cout<<"Data contains invalid characters"<<endl;
      continue;
    }
    //cout<<"Data is valid, continue"<<endl;
    s=string(buffer, received);
    if(s == config.stopcommand) {
      cout<<"Received stop command, exiting now"<<endl;
      break;
    }

    s=config.scriptsDir+"/"+s;
    cout<<"EXECUTING : "<<s<<endl;
    if(file_existence_tester(s)) {
      memset(buffer,0,sizeof(buffer));
      execToBuf(s,buffer,sizeof(buffer)-1);
      if(config.OutputSendRules == always)
        udp.respond(buffer, strlen(buffer));
    }
    else {
      cout<<"Error: file does not exist!"<<endl;
    }
    cout<<"EXECUTION COMPLETED"<<endl;


  }
  udp.cleanup();
}
