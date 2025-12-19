#include"Reactor.h"
#include<vector>

bool Reactor::AddEvent(Event& event)
{
	
	struct epoll_event ev;
	ev.data.ptr = &event;
	ev.events = event.event();

	int opt = (event.status() == 1) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

	pthread_mutex_lock(&_epmutex);
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

bool Reactor::DelEvent(Event& event)
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

bool Reactor::ModEvent(Event& event, const uint32_t type, int len, const Event::callback callback)
{
	event.set_event(type);
    event.set_len(len);
    event.set_handledata(callback);
	return true;
}

bool Reactor::SetEvent(Event& event)
{
	this->get_events()[event.fd()] = event;
	return true;
}

void Reactor::AddThreadPool(ThreadPool* threadpool)
{
    _threadpool = threadpool;
}

void Reactor::loop()
{
	if (_epfd == -1) {
		throw std::runtime_error("Reactor not initialized");
	}

	struct epoll_event active_events[1025];
	cout << "Events start looping" << endl;
	while (true)
	{
		int ready_num = epoll_wait(_epfd, active_events, 1024, -1);
		
		if (ready_num == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
			throw runtime_error("epoll_wait failed");
			return;
		} 

		for (int i = 0; i < ready_num; ++i) 
		{ 
			epoll_event& active_ev = active_events[i];
			Event* event = static_cast<Event*>(active_ev.data.ptr);
			if (active_ev.events & EPOLLIN && event->event() & EPOLLIN)
			{
				_threadpool->AddTask(Task(event->handledata(), event->fd(), event->arg()));
			}
			if (active_ev.events & EPOLLOUT && event->event() & EPOLLOUT) 
			{
				_threadpool->AddTask(Task(event->handledata(), event->fd(), event->arg()));
			}
		}
	}
}
