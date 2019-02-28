#ifndef UDPTOSCRIPT_CONFIG_HPP
#define UDPTOSCRIPT_CONFIG_HPP 1

#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

using namespace std;
enum OutputSendRules_t { never, always };
struct config_t {
  vector<string> globalAllowedIPS;
  uint16_t port;
  string stopcommand;
  string scriptsDir;
  OutputSendRules_t OutputSendRules;
};

int loadconfig(config_t &config);
int verifyconfig(config_t &config);
void destroyconfig(config_t &config);
#endif
