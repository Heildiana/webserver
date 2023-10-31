CXX=g++
CFLAGS=-std=c++11 -O2 -Wall -g
CXXFLAGS=-std=c++11 -O2 -Wall -g
CXXFLAGS_ = -std=c++11

TARGET:=myserver
SOURCE:=$(wildcard ../*.cpp)
OBJS= ./HttpConnection.cpp  ./MyEpoll.cpp ./webserver.cpp ./main.cpp

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS_)  $(OBJS) -o ./bin/$(TARGET) 