#pragma once
#include "ReactorThread.h"
#include <vector>
#include <memory>

class ReactorThreadPool {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;

    ReactorThreadPool(EventLoop* baseLoop) : baseLoop_(baseLoop), started_(false), numThreads_(0), next_(0) {}
    ~ReactorThreadPool() {}

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;  

    bool started_;
    int numThreads_;
    int next_;  
    vector<unique_ptr<ReactorThread>> threads_;
    vector<EventLoop*> loops_;
};