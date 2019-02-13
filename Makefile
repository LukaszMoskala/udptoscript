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
CXXFLAGS=-O3 --std=c++17 -lsfml-network
CFLAGS=-O3
PREFIX=/usr/local

all: udptoscript

udptoscript.o: udptoscript.cpp
	$(CXX) $(CXXFLAGS) -c -o udptoscript.o udptoscript.cpp
udptoscript: udptoscript.o
	$(CXX) $(CXXFLAGS) -o udptoscript udptoscript.o
