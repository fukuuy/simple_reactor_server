#pragma once
#include"EventLoop.h"
#include<sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include<cstring>
#include"ReactorThreadPool.h"

class Server
{
public:
	Server(const string& ip, int port) : ip(ip), port(port)
	{
		listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_fd < 0)
		{
			throw runtime_error(string("socket create error: ") + strerror(errno));
		}

		int opt = 1;
		setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(static_cast<uint16_t>(port));
		if (inet_pton(AF_INET, ip.c_str(), &saddr.sin_addr) <= 0)
		{
			close(listen_fd);
			throw runtime_error("inet_pton error");
		}
		cout << "Server is created" << endl;
	}
	
	explicit Server(int port) : Server("0.0.0.0", port) {}
	explicit Server(const string& ip) : Server(ip, 8080) {}

	~Server()
	{
		if (listen_fd != -1)
		{
			close(listen_fd);
			listen_fd = -1;
		}
	}
	
	bool start();
	bool AddListener(EventLoop* reactor, callback Accept_cb);
	void setThreadPool(ReactorThreadPool* pool) { _threadPool = pool; }

	int get_fd() { return listen_fd; }
	string get_ip() { return ip; }
	int get_port() { return port; }
    ReactorThreadPool* getThreadPool() { return _threadPool; }
	bool is_start() { return listen_fd != -1; }
	bool is_stop() { return listen_fd == -1; }
private:
	int listen_fd;

	string ip;
    int port;
	struct sockaddr_in saddr;

	ReactorThreadPool* _threadPool;
};

