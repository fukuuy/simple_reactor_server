#pragma once
#include<fcntl.h>
#include<cstring>
#include "Reactor.h"
#include "Event.h"

class Handler
{
public:
    static int handleAccpet(int listen_fd, void* arg);
    static int handleRecv(int fd, void* arg);
    static int handleSend(int fd, void* arg);
};

