#pragma once
#include"Event.h"
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<map>
#include<pthread.h>
#include<iostream>
#include"ThreadPool.h"

class Reactor
{
public:
	static constexpr int MAX_EVENT_NUM = 1024;

	Reactor()
	{
		_epfd = epoll_create(1);
		if (_epfd == -1) throw runtime_error("epoll_create error");
		pthread_mutex_init(&_epmutex, nullptr);
		_threadpool = nullptr;
	}

	~Reactor()
	{
		close(_epfd);
		pthread_mutex_destroy(&_epmutex);
	}

	Reactor(const Reactor&) = delete;
    Reactor& operator=(const Reactor&) = delete;

	bool AddEvent(Event& event);
    bool DelEvent(Event& event);
	bool ModEvent(Event& event, const uint32_t type, int len, const callback callback);
	bool SetEvent(Event& event);

	void AddThreadPool(ThreadPool* threadpool);
	void loop();

	map<int, Event>& get_events() { return _events; }

private:
	int _epfd;
	map<int, Event> _events;
	pthread_mutex_t _epmutex;
    ThreadPool* _threadpool;
};

