#include"EventLoop.h"
#include<vector>

bool EventLoop::addEvent(Event& event)
{
	struct epoll_event ev;
	ev.data.ptr = &event;
	ev.events = event.event();

	pthread_mutex_lock(&_evmutex);
	int opt = (event.status() == 1) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	int ret = epoll_ctl(_epfd, opt, event.fd(), &ev);
	pthread_mutex_unlock(&_evmutex);

	if (ret == -1)
	{
		throw runtime_error("epoll_ct1 error");
		return false;
	}

	event.set_status(1);
	return true;
}

bool EventLoop::delEvent(Event& event)
{
	if (event.status() != 1) return true;

	struct epoll_event ev;
	ev.data.ptr = &event;
	ev.events = event.event();

	pthread_mutex_lock(&_evmutex);
	int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, event.fd(), &ev);
	pthread_mutex_unlock(&_evmutex);

	if (ret == -1)
	{
		throw runtime_error("epoll_ct1 error");
		return false;
	}

	event.set_status(false);
	return true;
}

bool EventLoop::modEvent(Event& event, const uint32_t type, int len, const callback callback)
{
	event.set_event(type);
	event.set_len(len);
	event.set_handledata(callback);
	return true;
}

bool EventLoop::setEvent(Event& event)
{
	pthread_mutex_lock(&_evmutex);
	this->get_events()[event.fd()] = event;
	pthread_mutex_unlock(&_evmutex);
	return true;
}

bool EventLoop::removeEvent(Event& event)
{
	pthread_mutex_lock(&_evmutex);
	this->get_events().erase(event.fd());
	pthread_mutex_unlock(&_evmutex);
	return true;
}


void EventLoop::cleanTimeoutConnections() 
{
	pthread_mutex_lock(&_evmutex);
	vector<int> timeout_fds;
	for (auto& pair : _events) 
	{
		int fd = pair.first;
		Event& event = pair.second;
		if (Handler::isConnectionTimeout(event)) 
		{
			timeout_fds.push_back(fd);
		}
	}
	pthread_mutex_unlock(&_evmutex);
	for (int fd : timeout_fds) 
	{
		if (_events.count(fd)) 
		{
			Event& event = _events[fd];
			delEvent(event);
			event.close();
			removeEvent(event);
		}
	}
}

void EventLoop::loop()
{
	if (_epfd == -1)
	{
		throw runtime_error("EventLoop not initialized");
	}

	struct epoll_event active_events[MAX_EVENT_NUM];

	auto last_clean_time = chrono::system_clock::now();
	while (!_quit)
	{
		int ready_num = epoll_wait(_epfd, active_events, MAX_EVENT_NUM, 1000);

		if (ready_num == -1)
		{
			if (errno == EINTR) continue;
			throw runtime_error("epoll_wait failed");
			return;
		}

		auto now = chrono::system_clock::now();
		auto duration = chrono::duration_cast<chrono::seconds>(now - last_clean_time).count();
		if (duration >= 5) {
			cleanTimeoutConnections();
			last_clean_time = now;
		}

		for (int i = 0; i < ready_num; ++i)
		{
			epoll_event& active_ev = active_events[i];
			Event* event = static_cast<Event*>(active_ev.data.ptr);
			if (active_ev.events & EPOLLIN && event->event() & EPOLLIN)
			{
				event->handledata()(event->fd(), event->arg());
			}
			if (active_ev.events & EPOLLOUT && event->event() & EPOLLOUT)
			{
				event->handledata()(event->fd(), event->arg());
			}
		}
	}
}