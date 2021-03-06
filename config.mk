VERSION = 1.0.0

PREFIX = /usr/local

INCS = -I/usr/include/libevdev-1.0

CPPFLAGS = $(INCS) -DVERSION=\"$(VERSION)\"

COMPFLAGS = -pedantic -Wall -Wextra -O3
CFLAGS = $(COMPFLAGS) -std=c99
CXXFLAGS = $(COMPFLAGS) -std=c++11

LDFLAGS = -lstdc++ -lyaml-cpp -levdev

CC = cc
CXX = g++

