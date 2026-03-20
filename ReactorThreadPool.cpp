#include "ReactorThreadPool.h"
#include <assert.h>


void ReactorThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!_started);
    _started = true;

    for (int i = 0; i < _numThreads; ++i) 
    {
        _threads.emplace_back(new ReactorThread(cb));
        _loops.push_back(_threads.back()->startLoop());
    }

    if (_numThreads == 0 && cb) 
    {
        cb(baseLoop_);
    }
}

EventLoop* ReactorThreadPool::getNextLoop()
{
    assert(_started);
    EventLoop* loop = baseLoop_;

    if (!_loops.empty()) 
    {
        loop = _loops[_next];
        ++_next;
        if (_next >= _loops.size())  _next = 0;
    }
    return loop;
}