#include"Server.h"
#include"Handler.h"

int main()
{
	Server server("192.168.80.128", 10000);
    EventLoop baseloop; 
    ReactorThreadPool threadPool(&baseloop);

    threadPool.setThreadNum(3);
    threadPool.start();

    server.setThreadPool(&threadPool);

    server.AddListener(&baseloop, Handler::handleAccpet);
    server.start();
    baseloop.loop();

	return 0;
}