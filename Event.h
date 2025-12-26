#pragma once
#include<chrono>
#include<cstring>
#include<unistd.h>
#include<functional>

using namespace std;
using callback = function<int(int, void*)>;


class Event
{
public:
	static constexpr int MAX_BUFFER_SIZE = 1024;

	inline void close()
		{
			if (_fd != -1)
			{
				::close(_fd);
				_fd = -1;
			}
		}
	inline void update_active() { _last_active = chrono::system_clock::now(); }  

	Event() : _fd(-1), _status(0), _len(0), _handledata(nullptr), _arg(nullptr)  
	{
		memset(_buffer, 0, MAX_BUFFER_SIZE);
		update_active();
	}

	~Event()
	{
		this->close();
	}

	uint32_t event() const { return _event; }
	bool status() const { return _status; }
	int fd() const { return _fd; }
	char* buffer() { return _buffer; }
	void* arg() const { return _arg; }
	int len() const { return _len; }
	callback handledata() const { return _handledata; }  
	chrono::system_clock::time_point last_active() const { return _last_active; } 

	void set_fd(const int fd) { _fd = fd; }
	void set_len(const int len) { _len = len; }
	void set_event(const uint32_t event) { _event = event; }
	void set_status(const bool status) { _status = status; }
	void set_handledata(const callback& cb) { _handledata = cb; }
	void set_arg(void* arg) { _arg = arg; }
    

	void set(int fd, uint32_t event, callback cb, void* arg)
	{
		_fd = fd;
		_event = event;
		_arg = arg;
		_handledata = cb;
		_len = 0;
		memset(_buffer, 0, MAX_BUFFER_SIZE);
		update_active();
	}

private:

	int _fd;
	uint32_t _event;

	bool _status;

	char _buffer[MAX_BUFFER_SIZE];
	int _len;

	callback _handledata;
	void* _arg;

	chrono::system_clock::time_point _last_active;
};

