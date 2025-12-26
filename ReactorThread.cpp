#include "ReactorThread.h"
#include <assert.h>

EventLoop* ReactorThread::startLoop()
{
    assert(loop_ == nullptr);
    pthread_create(&threadId_, nullptr, threadFunc, this);

    while (loop_ == nullptr) {
        usleep(100);
    }

    return loop_;
}

void* ReactorThread::threadFunc(void* arg)
{
    ReactorThread* thread = static_cast<ReactorThread*>(arg);
    thread->runInThread();
    return nullptr;
}

void ReactorThread::runInThread() 
{
    EventLoop loop;

    if (initCallback_) {
        initCallback_(&loop);
    }

    loop_ = &loop;
    loop.loop();
    loop_ = nullptr;
}