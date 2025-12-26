#pragma once
#include"Event.h"
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<map>
#include<pthread.h>
#include<iostream>

class EventLoop
{
public:
	static constexpr int MAX_EVENT_NUM = 1024;

	EventLoop() : _quit(false) {
		_epfd = epoll_create(1);
		if (_epfd == -1) throw runtime_error("epoll_create error");
		pthread_mutex_init(&_epmutex, nullptr);
		_threadId = pthread_self();  
	}

	~EventLoop()
	{
		close(_epfd);
		pthread_mutex_destroy(&_epmutex);
	}


	bool AddEvent(Event& event);
    bool DelEvent(Event& event);
	bool ModEvent(Event& event, const uint32_t type, int len, const callback callback);
	bool SetEvent(Event& event);

	void loop();

	map<int, Event>& get_events() { return _events; }

	void quit() { _quit = true; }
	void assertInLoopThread() {
		if (pthread_self() != _threadId) {
			
		}
	}
private:
	int _epfd;
	map<int, Event> _events;
	pthread_mutex_t _epmutex;
	pthread_t _threadId;
	bool _quit;
};

