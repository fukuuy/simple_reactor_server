#include "Handler.h"
#include "Server.h"
#include "HttpParser.h"

int Handler::handleRecv(int fd, void* arg)
{
    EventLoop* reactor = static_cast<EventLoop*>(arg);
    Event& event = reactor->get_events()[fd];

    if (isConnectionTimeout(event)) {
        reactor->delEvent(event);
        event.close();
        reactor->removeEvent(event);
        return -1;
    }

    int total_len = 0;
    while (true)
    {
        int len = recv(fd, event.buffer() + total_len, Event::MAX_BUFFER_SIZE - total_len + 1, 0);
        if (len < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            else
            {
                reactor->delEvent(event);
                event.close();
                reactor->removeEvent(event);
                //cout << "recv error on fd " << fd << ": " << strerror(errno) << endl;
                return -1;
            }
        }
        else if (len == 0)
        {
            //cout << "Close connect: [fd: " << fd << "]" << endl;
            reactor->delEvent(event);
            event.close();
            reactor->removeEvent(event);
            return 0;
        }
        else
        {
            total_len += len;
            if (total_len >= Event::MAX_BUFFER_SIZE - 1) break;
        }
    }

    if (total_len > 0)
    {
        event.buffer()[total_len] = '\0';
        event.set_len(total_len);
        //cout << "Receive [fd: " << fd << "] data:\n"<< event.buffer() << endl;

        // 解析HTTP请求
        HttpParser::HttpRequest req;
        if (HttpParser::parseRequest(event.buffer(), req))
        {
            string html_path = (req.path == "/" || req.path.empty()) ? "./index.html" : "." + req.path;
            string html_content = HttpParser::readHtmlFile(html_path);
            int status_code = (html_content.find("404") != string::npos) ? 404 : 200;

            string response = HttpParser::buildResponse(status_code, html_content);

            strncpy(event.buffer(), response.c_str(), response.size());
            event.set_len(response.size());
        }
        else
        {
            string error_resp = HttpParser::buildResponse(400, "<h1>400 Bad Request</h1>");
            strncpy(event.buffer(), error_resp.c_str(), error_resp.size());
            event.set_len(error_resp.size());
        }
        //解析完成

        event.update_active();
        reactor->modEvent(event, EPOLLOUT | EPOLLET, event.len(), Handler::handleSend);
        reactor->addEvent(event);
    }

    return total_len;
}

int Handler::handleSend(int fd, void* arg)
{
    EventLoop* reactor = static_cast<EventLoop*>(arg);
    Event& event = reactor->get_events()[fd];

    if (isConnectionTimeout(event)) {
        reactor->delEvent(event);
        event.close();
        reactor->removeEvent(event);
        return -1;
    }

    reactor->delEvent(event);

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
                reactor->modEvent(event, EPOLLOUT | EPOLLET, remain_len, Handler::handleSend);
                reactor->addEvent(event);

                return total_sent;
            }
            else
            {
                //cout << "Send error on fd: " << fd << ":\n" << strerror(errno) << endl;
                event.close();
                reactor->removeEvent(event);
                return -1;
            }
        }
        else if (len == 0)
        {
            //cout << "Closed connection [fd: " << fd << "]" << endl;
            event.close();
            reactor->removeEvent(event);
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
        //cout << "Send [fd: " << fd << "] data:\n " << event.buffer() << endl;
    }

    event.update_active();
    event.set_len(0);
    memset(event.buffer(), 0, Event::MAX_BUFFER_SIZE + 1);

    reactor->modEvent(event, EPOLLIN | EPOLLET, 0, Handler::handleRecv);
    reactor->addEvent(event);

    return total_sent;
}


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
                //cout << "accept interrupted by signal, retrying" << endl;
                continue;
            }
            else
            {
                throw runtime_error(string("accept error: ") + strerror(errno));
                break;
            }
        }

        int flags = fcntl(cfd, F_SETFL, O_NONBLOCK);
        if (flags == -1)
        {
            close(cfd);
            throw runtime_error(string("set fd nonblock error") + strerror(errno));
        }

        EventLoop* ioLoop = threadPool->getNextLoop();

        Event& event = ioLoop->get_events()[cfd];
        event.set(cfd, EPOLLIN | EPOLLET, handleRecv, ioLoop);
        if (!ioLoop->addEvent(event))
        {
            close(cfd);
            //cout << "Failed to add event for fd: " << cfd << endl;
            continue;
        }

        //cout << "New connect: [IP: " << inet_ntoa(caddr.sin_addr) << ", port:" << ntohs(caddr.sin_port) << "]" << endl;
    }
    return 1;
}