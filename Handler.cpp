#include "Handler.h"
#include "Server.h"

int Handler::handleAccpet(int listen_fd, void* arg)
{
    Server* server = static_cast<Server*>(arg);
    ReactorThreadPool* threadPool = server->getThreadPool();
    sockaddr_in caddr;
    socklen_t len = sizeof(caddr);

    while (true) 
    {
        int cfd = accept(listen_fd, (sockaddr*)&caddr, &len);
        if (cfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            else if (errno == EINTR) 
            {
                cout << "accept interrupted by signal, retrying..." << endl;
                continue;
            }
            else
            {
                throw runtime_error(string("accept error: ") + strerror(errno));
                break;
            }
        }

        int flags = fcntl(cfd, F_SETFL, O_NONBLOCK);
        if (flags == -1) {
            close(cfd);
            throw runtime_error(string("set fd nonblock error") + strerror(errno));
        }

        EventLoop* ioLoop = threadPool->getNextLoop();

        Event& event = ioLoop->get_events()[cfd];
        event.set(cfd, EPOLLIN | EPOLLET, handleRecv, ioLoop);
        if (!ioLoop->AddEvent(event)) {
            close(cfd);
            cout << "Failed to add event for fd: " << cfd << endl;
            continue;
        }

        cout << "New connect: [IP: " << inet_ntoa(caddr.sin_addr)
            << ", port:" << ntohs(caddr.sin_port) << "]" << endl;
    }
    return 1;
}

int Handler::handleRecv(int fd, void* arg)
{
    EventLoop* reactor = static_cast<EventLoop*>(arg);
    Event& event = reactor->get_events()[fd];

    int total_len = 0;
    while (true) {
        int len = recv(fd, event.buffer() + total_len, Event::MAX_BUFFER_SIZE - total_len, 0);
        if (len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            else {
                event.close();
                cout << "recv error on fd " << fd << ": " << strerror(errno) << endl;
                return -1;
            }
        }
        else if (len == 0) {
            cout << "Close connect: [fd: " << fd << "]" << endl;
            reactor->DelEvent(event);
            event.close();
            return 0;
        }
        else {
            total_len += len;
            if (total_len >= Event::MAX_BUFFER_SIZE - 1) break;
        }
    }

    if (total_len > 0) {
        event.buffer()[total_len] = '\0';
        event.set_len(total_len);
        cout << "Receive [fd: " << fd << "] data : "<< event.buffer() << endl;

        event.update_active();
        reactor->ModEvent(event, EPOLLOUT | EPOLLET, total_len, Handler::handleSend);
        reactor->AddEvent(event);
    }

    return total_len;
}

int Handler::handleSend(int fd, void* arg)
{
    EventLoop* reactor = static_cast<EventLoop*>(arg);
    Event& event = reactor->get_events()[fd];

    reactor->DelEvent(event);

    int total_sent = 0;
    int remain_len = event.len();
    const char* buffer = event.buffer();

    while (remain_len) 
    {
        int len = send(fd, buffer + total_sent, remain_len, 0);
        if (len < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                event.set_len(remain_len);
                memmove(event.buffer(), buffer + total_sent, remain_len);
                event.buffer()[remain_len] = '\0';

                event.update_active();
                reactor->ModEvent(event, EPOLLOUT | EPOLLET, remain_len, Handler::handleSend);
                reactor->AddEvent(event);

                return total_sent;
            }
            else
            {
                cout << "Send error on fd: " << fd << ": " << strerror(errno) << endl;
                event.close();
                return -1;
            }
        }
        else if (len == 0) 
        {
            cout << "Closed connection [fd: " << fd << "]" << endl;
            event.close();
            return 0;
        }
        else
        {
            total_sent += len;
            remain_len -= len;
        }
    }

    if (total_sent > 0)
    {
        cout << "Send [fd: " << fd << "] data: " << event.buffer() << endl;
    }

    event.update_active();
    reactor->ModEvent(event, EPOLLIN | EPOLLET, 0, Handler::handleRecv);
    reactor->AddEvent(event);

    return total_sent;
}
