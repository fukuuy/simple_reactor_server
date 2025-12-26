#pragma once
#include "EventLoop.h"
#include <pthread.h>
#include <functional>

class ReactorThread {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;

    ReactorThread(const ThreadInitCallback& cb)
        : loop_(nullptr), exiting_(false), initCallback_(cb) {
    }

    ~ReactorThread() {
        exiting_ = true;
        if (loop_) {
            loop_->quit();
            pthread_join(threadId_, nullptr);
        }
    }

    EventLoop* startLoop();

private:
    static void* threadFunc(void* arg);
    void runInThread();

    EventLoop* loop_;
    bool exiting_;
    pthread_t threadId_;
    ThreadInitCallback initCallback_;
};