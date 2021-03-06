#Copyright (C) 2019 Łukasz Konrad Moskała
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <https://www.gnu.org/licenses/>.

CC=gcc
CXX=g++
CXXFLAGS=-O3 --std=c++17
CFLAGS=-O3
PREFIX=/usr/local

all: md udptoscript

md:
	mkdir -p bin obj

obj/network.o: src/network.cpp src/network.hpp
	$(CXX) $(CXXFLAGS) -c -o obj/network.o src/network.cpp

obj/udptoscript.o: src/udptoscript.cpp
	$(CXX) $(CXXFLAGS) -c -o obj/udptoscript.o src/udptoscript.cpp

obj/config.o: src/config.cpp src/config.hpp
	$(CXX) $(CXXFLAGS) -c -o obj/config.o src/config.cpp

udptoscript: obj/udptoscript.o obj/config.o obj/network.o src/config.hpp src/network.hpp
	$(CXX) $(CXXFLAGS) -o bin/udptoscript obj/udptoscript.o obj/config.o obj/network.o

install: udptoscript
	install -m 775 bin/udptoscript $(PREFIX)/bin/
	@echo use make install-service to install systemd service
install-service:
	install -m 664 src/udptoscript.service /etc/systemd/system/
	@echo ===========================================================
	@echo = YOU SHOULD EDIT /etc/systemd/system/udptoscript.service =
	@echo =       AND CHANGE USER AS WHICH DAEMON IS RUNNING        =
	@echo ===========================================================
clean:
	rm -f obj/*.o
