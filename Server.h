#pragma once
#include"Reactor.h"
#include<sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include<cstring>

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
		saddr.sin_port = htons(port);
		if (inet_pton(AF_INET, ip.c_str(), &saddr.sin_addr) <= 0)
		{
			close(listen_fd);
			throw std::runtime_error("inet_pton error");
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
	bool AddReactor(Reactor* reactor, Event::callback Accept_cb);

	inline int get_fd() { return listen_fd; }
	inline string get_ip() { return ip; }
	inline int get_port() { return port; }
	inline bool is_start() { return listen_fd != -1; }
	inline bool is_stop() { return listen_fd == -1; }
private:
	int listen_fd;

	string ip;
    int port;
	struct sockaddr_in saddr;
};

