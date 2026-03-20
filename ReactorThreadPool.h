#pragma once
#include "ReactorThread.h"
#include <vector>
#include <memory>

class ReactorThreadPool {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;

    ReactorThreadPool(EventLoop* baseLoop) : baseLoop_(baseLoop), _started(false), _numThreads(0), _next(0) {}
    ~ReactorThreadPool() {}

    //设置线程池的线程数量
    void setThreadNum(int numThreads) { _numThreads = numThreads; }
    //启动线程池
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    //轮询获取下一个事件循环
    EventLoop* getNextLoop();

private:
    //主线程的监听套接字事件循环
    EventLoop* baseLoop_;  

    //线程池启动状态标记
    bool _started;
    //线程池的工作线程数量
    int _numThreads;
    //轮询索引
    int _next;
    //子线程列表
    vector<unique_ptr<ReactorThread>> _threads;
    //子线程事件循环列表
    vector<EventLoop*> _loops;
};