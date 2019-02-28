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
//configuration structure
config_t config;
//Input and output buffer
char buffer[1024];
//Udp server instance
UdpServer udp;

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
//config.pidfile may change at runtime, so we have to save these settings somewhere
bool haveICreatedPidFile=false;
string createdPidFileName="";
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
      if(haveICreatedPidFile)
        unlink(createdPidFileName.c_str());
      exit(1);
    }
    if(verifyconfig(config)) {
      //config is invalid, exit
      udp.cleanup();
      if(haveICreatedPidFile)
        unlink(createdPidFileName.c_str());
      exit(1);
    }
  }
  if(signum == SIGTERM) {
    cout<<"received SIGTERM, exiting gracefully"<<endl;
    udp.cleanup();
    if(haveICreatedPidFile)
      unlink(createdPidFileName.c_str());
    exit(0);
  }
  //^C sends SIGINT
  if(signum == SIGINT) {
    cout<<"received SIGINT, exiting gracefully"<<endl;
    udp.cleanup();
    if(haveICreatedPidFile)
      unlink(createdPidFileName.c_str());
    exit(0);
  }
}
int main() {
  //load configuration
  if(loadconfig(config))
    return 1;
  //and verify it
  if(verifyconfig(config))
    return 1;
  //two functions above returns 1 on critical failure
  //and then we exit. Config library prints error messages
  //on it's own

  //handle signals
  signal(SIGUSR1, my_signal_handler);
  signal(SIGTERM, my_signal_handler);
  signal(SIGINT, my_signal_handler);

  //write pidfile
  if(config.pidfile != "") {
    if(file_existence_tester(config.pidfile)) {
      //pidfile exists, read it and check if process is running
      cout<<"ERROR: pidfile "<<config.pidfile<<" exists!"<<endl;
      cout<<"If you are sure that process that created it is not running, delete it"<<endl;
      return 1;
    }
    else {
      ofstream pidfile;
      pidfile.open(config.pidfile.c_str());
      if(!pidfile.is_open()) {
        cout<<"WARNING: Failed to write pidfile "<<config.pidfile<<endl;
      }
      else {
        pidfile<<getpid();
        pidfile.close();
        haveICreatedPidFile=true;
        //because config.pidfile can be changed at runtime
        createdPidFileName=config.pidfile;
      }
    }
  }
  //Initialize network connectivity
  //this function takes only port number and ip to listen at as argument
  //in case of failure, it generates error message to stdout
  //and returns non-zero exit code
  if(udp.begin(config.port, config.listenIP))
    return 1;

  //infinite loop that handles server
  //only one script can be executed at a time
  //however, you can run multiple daemons (on diffrent ports, of course)
  //to overcome that limitation
  while( 1 ) {
    //zero out buffer to avoid using command from previous client
    //prabably doesn't matter anyway, but it's good practise
    memset(buffer, 0, sizeof(buffer));
    //read data from client.
    //only up to SIZEOF(BUF) bytes can be read, and at this moment is's size is
    //1024 bytes. however, standard ehternet connection has MTU of about 1500
    //and any packet can't exceed that, so that's upper limit anyway
    //but you shouldn't have script names THAT long ;)
    //in any case, symlinks are your friends
    int received=udp.read(buffer, sizeof(buffer));
    //if length is 0 or error has occured, skip that packet
    //error message is already printed by udp.read
    if(received < 1)
      continue;
    //this variable means that IP that tries to connect is on allowed list
    bool allowed=false;
    //and that's checked here
    for(int i=0;i<config.globalAllowedIPS.size() && !allowed;i++) {
      allowed= ( config.globalAllowedIPS[i] == udp.getClientIP() );
    }
    if(!allowed) {
      //if we'r here, IP is not allowed
      if(config.errorMessages == always) {
        udp.respond("Your IP is not white-listed on this server.");
      }
      cout<<"IP "<<udp.getClientIP()<<" not in allowed list"<<endl;
      continue;
    }
    bool possiblygood=true;
    //verifies if message contains only letters and numbers, or dot
    //that's for safety, so user can't do '../../../../../bin/ls'
    //or something like that
    for(int i=0;i<received && possiblygood;i++) {
      possiblygood=( ( buffer[i] >= '0' && buffer[i] <= '9' ) ||
                     ( buffer[i] >= 'a' && buffer[i] <= 'z' ) ||
                     ( buffer[i] >= 'A' && buffer[i] <= 'Z' ) ||
                       buffer[i] == '.');
    }

    if(!possiblygood) {
      if(config.errorMessages == always) {
        udp.respond("Messages can only contain numbers, letters and dot.");
      }
      cout<<"Data contains invalid characters"<<endl;
      continue;
    }
    string s=string(buffer, received);
    if(s == config.stopcommand) {

      udp.respond("Exiting now.");
      cout<<"Received stop command, exiting now"<<endl;
      break;
    }

    s=config.scriptsDir+"/"+s;
    cout<<"EXECUTING : "<<s<<endl;
    if(file_existence_tester(s)) {
      //clear buffer before sending output back
      //output is limited to sizeof(buffer)-1
      //it shouldn't be really a problem
      //also, output is expected to be text, not binary, as NULL is used to
      //locate end of string
      memset(buffer,0,sizeof(buffer));
      //execute and get output
      execToBuf(s,buffer,sizeof(buffer)-1);
      //if we should send output back, do it
      if(config.OutputSendRules == always)
        udp.respond(buffer, strlen(buffer));
    }
    else {
      if(config.errorMessages == always) {
        udp.respond("Script does not exist.");
      }
      cout<<"Error: file does not exist!"<<endl;
    }
    cout<<"EXECUTION COMPLETED"<<endl;


  }
  udp.cleanup();
  if(haveICreatedPidFile)
    unlink(createdPidFileName.c_str());
}
