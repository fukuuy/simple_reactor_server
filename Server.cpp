#include "Server.h"

bool Server::start()
{
    if (bind(listen_fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
    {
        close(listen_fd);
        throw runtime_error(string("bind error: ") + strerror(errno));
        return false;
    }

    if (listen(listen_fd, 128) < 0)
    {
        close(listen_fd);
        throw runtime_error(string("listen error") + strerror(errno));
        return false;
    }
    cout << "Server start listening [IP: " << ip << "; Port: " << port << "]" << endl;
    return true;
}

bool Server::AddListener(EventLoop* reactor, callback Accept_cb)
{
    if (listen_fd == -1) return false;
    int flags = fcntl(listen_fd, F_SETFL, O_NONBLOCK);
    if (flags == -1) {
        close(listen_fd);
        throw runtime_error("fcntl set nonblock error");
    }
    Event& event = reactor->get_events()[listen_fd];
    event.set(listen_fd, EPOLLIN | EPOLLET, Accept_cb, this);
    reactor->AddEvent(event);
    return true;
}