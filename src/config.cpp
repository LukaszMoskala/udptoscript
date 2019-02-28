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
int loadconfig(config_t &config) {
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
  return 0; //no errors
}
int verifyconfig(config_t &config) {
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
  return 0; //no errors
}
void destroyconfig(config_t &config) {
  config.port=0;
  config.stopcommand="";
  config.scriptsDir="";
  config.OutputSendRules=never;
  config.globalAllowedIPS.clear();
}
