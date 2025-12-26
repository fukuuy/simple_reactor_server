#include"EventLoop.h"
#include<vector>

bool EventLoop::AddEvent(Event& event)
{
	
	struct epoll_event ev;
	ev.data.ptr = &event;
	ev.events = event.event();

	pthread_mutex_lock(&_epmutex);
	int opt = (event.status() == 1) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	int ret = epoll_ctl(_epfd, opt, event.fd(), &ev);
	pthread_mutex_unlock(&_epmutex);

	if (ret == -1)
	{
		throw runtime_error("epoll_ct1 error");
		return false;
	}

	event.set_status(1);
	return true;
}

bool EventLoop::DelEvent(Event& event)
{
	if (event.status() != 1) return true;

	struct epoll_event ev;
	ev.data.ptr = &event;
	ev.events = event.event();

	pthread_mutex_lock(&_epmutex);
	int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, event.fd(), &ev);
	pthread_mutex_unlock(&_epmutex);

	if (ret == -1) {
		throw runtime_error("epoll_ct1 error");
		return false;
	}

	event.set_status(false);
	return true;
}

bool EventLoop::ModEvent(Event& event, const uint32_t type, int len, const callback callback)
{
	event.set_event(type);
    event.set_len(len);
    event.set_handledata(callback);
	return true;
}

bool EventLoop::SetEvent(Event& event)
{
	this->get_events()[event.fd()] = event;
	return true;
}


void EventLoop::loop() {
	if (_epfd == -1) {
		throw runtime_error("EventLoop not initialized");
	}

	struct epoll_event active_events[MAX_EVENT_NUM];
	while (!_quit)  
	{
		int ready_num = epoll_wait(_epfd, active_events, MAX_EVENT_NUM, -1);

		if (ready_num == -1) {
			if (errno == EINTR) continue;
			throw runtime_error("epoll_wait failed");
			return;
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
