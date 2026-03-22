#include"Server.h"
#include"Handler.h"
#include <thread>
int main()
{
	Server server("0.0.0.0", 10000);
    EventLoop baseloop; 
    ReactorThreadPool threadPool(&baseloop);

    threadPool.setThreadNum(thread::hardware_concurrency());
    server.setThreadPool(&threadPool);
    server.start();
    server.AddListener(&baseloop, Handler::handleAccpet);
    threadPool.start();
    baseloop.loop();

	return 0;
}