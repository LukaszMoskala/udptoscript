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
#include <fstream>
#include <vector>
#include <SFML/Network.hpp>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
using namespace std;

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
enum OutputSendRules_t { never, always };
struct config_t {
  vector<string> globalAllowedIPS;
  uint16_t port;
  string stopcommand;
  string scriptsDir;
  OutputSendRules_t OutputSendRules;
} config;

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

int main() {

  ifstream configfile;
  configfile.open("udptoscript.conf");
  if(!configfile.is_open()) {
    cout<<"Error: failed to open config file!"<<endl;
    return 1;
  }
  while(configfile.good()) {
    string currentline;
    getline(configfile, currentline);
    if(currentline.length() == 0 || currentline[0] == '#' || currentline[0] == ' ' || currentline[0] == '\t')
    continue;
    int spacepos=currentline.find(" ");
    if(spacepos == -1)
    continue;
    string param=currentline.substr(0, spacepos);
    string val=currentline.substr(spacepos+1);
    cout<<"Config: "<<param<<"="<<val<<endl;
    if(param == "port")
    config.port = atoi( val.c_str() );
    if(param == "allow")
    config.globalAllowedIPS.push_back(val);
    if(param == "stopcommand")
    config.stopcommand = val;
    if(param == "scriptsDir")
    config.scriptsDir = val;
    if(param == "outputsendrules") {
      if(val == "always")
        config.OutputSendRules = always;
      if(val == "never")
        config.OutputSendRules = never;
    }
  }
  configfile.close();

  //verify config

  if(config.scriptsDir.length() == 0) {
    cout<<"ERROR: no scriptsDir directive in config, refusing to work!"<<endl;
    return 1;
  }
  if(config.scriptsDir[0] != '/') {
    cout<<"WARNING: scriptsDir is not an absolute path!"<<endl;
  }
  if( config.scriptsDir[ config.scriptsDir.length() - 1 ] == '/' ) {
    config.scriptsDir = config.scriptsDir.substr(0, config.scriptsDir.length() - 1 ); //strip / from end of path
  }
  if(config.globalAllowedIPS.size() == 0) {
    cout<<"Warning: no allowed IPs specified, nobody will be able to connect!"<<endl;
  }
  if(!geteuid()) { //running as root is NOT recommended
    cout<<"WARNING! RUNNING AS ROOT IS NOT RECOMMENDED!"<<endl;
    cout<<"         Consider switching to unprivileged user and use sudo"<<endl;
    cout<<"         without password in your scripts!"<<endl;
  }
  if(config.port < 1024 && geteuid()) {
    cout<<"WARINING: config.port < 1024 and i'm not running as root"<<endl;
    cout<<"          bind() will probably fail, but i'll try anyway"<<endl;
    cout<<"          consider changing port to something greater than"<<endl;
    cout<<"          1024, because running as root is not recommended"<<endl;
  }
  if(config.stopcommand == "") {
    config.stopcommand = "stopdaemon";
    cout<<"WARNING: no stopcommand directive in config, using stopdaemon"<<endl;
  }

  sf::UdpSocket socket;
  socket.bind(config.port);

  char buffer[1024];
  std::size_t received = 0;
  sf::IpAddress sender;
  unsigned short port;
  string s="";

  while( s != config.stopcommand ) {
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

    s=config.scriptsDir+"/"+s;
    cout<<s<<endl;
    if(file_existence_tester(s)) {
      memset(buffer,0,sizeof(buffer));
      execToBuf(s,buffer,sizeof(buffer)-1);
      socket.send(buffer,strlen(buffer),sender,port);

    }


  }

}
