#pragma once
#include<queue>
#include<pthread.h>
#include<iostream>
#include<functional>

using namespace std;
using callback = function<int(int, void*)>;

class Task
{
public:
	
	callback function;
	void* arg;
	int fd;

	Task()
	{
        fd = -1;
		function = nullptr;
		arg = nullptr;
	}
	Task(callback func, int fd, void* arg)
	{
		this->function = func;
		this->arg = arg;
		this->fd = fd;
	}
};

class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();

	//添加任务
	void AddTask(Task task);
	void AddTask(callback func, int fd, void* arg);
	//取出任务
	Task TakeTask();

	//获取当前任务个数
	inline size_t GetTaskNumber()
	{
		return taskQ.size();
	}

private:
	queue<Task> taskQ;
	pthread_mutex_t mutexT;
};

