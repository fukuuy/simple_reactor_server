#pragma once
#include"Event.h"
#include"Handler.h"
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
		pthread_mutex_init(&_evmutex, nullptr);
		_threadId = pthread_self();
	}

	~EventLoop()
	{
		close(_epfd);
		pthread_mutex_destroy(&_evmutex);
	}

	//将事件添加到循环中
	bool addEvent(Event& event);
	//从循环中删除事件
	bool delEvent(Event& event);
	//修改事件的监听类型、数据长度和回调函数
	bool modEvent(Event& event, const uint32_t type, int len, const callback callback);
	//将事件存入事件映射表中
	bool setEvent(Event& event);
	//从事件映射表中删除指定事件
	bool removeEvent(Event& event);
	//事件循环
	void loop();

	map<int, Event>& get_events() { return _events; }

	void quit() { _quit = true; }

	void cleanTimeoutConnections();
private:
	//epoll 实例的文件描述符
	int _epfd;
	//事件映射表，key=fd，value=关联的Event对象
	map<int, Event> _events;

	//保证epoll_ctl线程安全
	pthread_mutex_t _evmutex;
	//事件循环所属线程ID
	pthread_t _threadId;
	bool _quit;
};

