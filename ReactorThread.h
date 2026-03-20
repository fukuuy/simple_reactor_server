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

    //创建线程并等待事件循环初始化完成
    EventLoop* startLoop();

private:
    static void* threadFunc(void* arg);
    //创建事件循环
    void runInThread();

    EventLoop* loop_;
    bool exiting_;
    pthread_t threadId_;
    ThreadInitCallback initCallback_;
};