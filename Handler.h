#pragma once
#include<fcntl.h>
#include<cstring>
#include "EventLoop.h"
#include "Event.h"

class Handler
{
public:
    //处理连接事件，分配子循环
    static int handleAccpet(int listen_fd, void* arg);
    //处理读事件
    static int handleRecv(int fd, void* arg);
    //处理写事件
    static int handleSend(int fd, void* arg);
};

