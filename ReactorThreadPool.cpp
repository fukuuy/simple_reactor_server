#include "ReactorThreadPool.h"
#include <assert.h>



void ReactorThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();  // 假设添加了线程检查方法

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        threads_.emplace_back(new ReactorThread(cb));
        loops_.push_back(threads_.back()->startLoop());
    }

    // 如果没有线程，主线程自己处理所有事件
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop* ReactorThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        // 轮询选择下一个EventLoop
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}