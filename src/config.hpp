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
